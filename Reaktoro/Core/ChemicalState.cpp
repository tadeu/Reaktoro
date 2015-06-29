// Reaktoro is a C++ library for computational reaction modelling.
//
// Copyright (C) 2014 Allan Leal
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

#include "ChemicalState.hpp"

// C++ includes
#include <iomanip>
#include <iostream>

// Reaktoro includes
#include <Reaktoro/Common/Constants.hpp>
#include <Reaktoro/Common/Exception.hpp>
#include <Reaktoro/Common/StringUtils.hpp>
#include <Reaktoro/Common/Units.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>
#include <Reaktoro/Core/Utils.hpp>
#include <Reaktoro/Thermodynamics/Water/WaterConstants.hpp>

namespace Reaktoro {
namespace {

auto errorNonAmountOrMassUnits(std::string units) -> void
{
    Exception exception;
    exception.error << "Cannot set the amount of the species.";
    exception.reason << "The provided units `" << units << "` is not convertible to units of amount or mass (e.g., mol and kg).";
    RaiseError(exception);
}

} // namespace

struct ChemicalState::Impl
{
    /// The chemical system instance
    ChemicalSystem system;

    /// The temperature state of the chemical system (in units of K)
    double T = 298.15;

    /// The pressure state of the chemical system (in units of Pa)
    double P = 1.0e+05;

    /// The molar amounts of the chemical species
    Vector n;

    /// The Lagrange multipliers with respect to the balance constraints (in units of J/mol)
    Vector y;

    /// The Lagrange multipliers with respect to the bound constraints of the species (in units of J/mol)
    Vector z;

    /// Construct a default ChemicalState::Impl instance
    Impl()
    {}

    /// Construct a custom ChemicalState::Impl instance
    Impl(const ChemicalSystem& system)
    : system(system)
    {
        n = zeros(system.numSpecies());
        y = zeros(system.numElements());
        z = zeros(system.numSpecies());
    }
};

ChemicalState::ChemicalState()
: pimpl(new Impl())
{}

ChemicalState::ChemicalState(const ChemicalSystem& system)
: pimpl(new Impl(system))
{}

ChemicalState::ChemicalState(const ChemicalState& other)
: pimpl(new Impl(*other.pimpl))
{}

ChemicalState::~ChemicalState()
{}

auto ChemicalState::operator=(ChemicalState other) -> ChemicalState&
{
    pimpl = std::move(other.pimpl);
    return *this;
}

auto ChemicalState::setTemperature(double val) -> void
{
    Assert(val > 0.0, "Cannot set temperature of the chemical state with a non-positive value.", "");
    pimpl->T = val;
}

auto ChemicalState::setTemperature(double val, std::string units) -> void
{
    setTemperature(units::convert(val, units, "kelvin"));
}

auto ChemicalState::setPressure(double val) -> void
{
    Assert(val > 0.0, "Cannot set pressure of the chemical state with a non-positive value.", "");
    pimpl->P = val;
}

auto ChemicalState::setPressure(double val, std::string units) -> void
{
    setPressure(units::convert(val, units, "pascal"));
}

auto ChemicalState::setSpeciesAmounts(double val) -> void
{
    Assert(val >= 0.0,
        "Cannot set the molar amounts of the species.",
        "The given molar abount is negative.");
    pimpl->n.fill(val);
}

auto ChemicalState::setSpeciesAmounts(const Vector& n) -> void
{
    Assert(static_cast<unsigned>(n.rows()) == system().numSpecies(),
        "Cannot set the molar amounts of the species.",
        "The dimension of the molar abundance vector "
        "is different than the number of species.");
    pimpl->n = n;
}

auto ChemicalState::setSpeciesAmounts(const Vector& n, const Indices& indices) -> void
{
    Assert(static_cast<unsigned>(n.rows()) == indices.size(),
        "Cannot set the molar amounts of the species with given indices.",
        "The dimension of the molar abundance vector "
        "is different than the number of indices.");
    rows(pimpl->n, indices) = n;
}

auto ChemicalState::setSpeciesAmount(Index index, double amount) -> void
{
    const unsigned num_species = system().species().size();
    Assert(amount >= 0.0,
        "Cannot set the molar amount of the species.",
        "The given molar amount is negative.");
    Assert(index < num_species,
        "Cannot set the molar amount of the species.",
        "The given index is out-of-range.");
    pimpl->n[index] = amount;
}

auto ChemicalState::setSpeciesAmount(std::string species, double amount) -> void
{
    const Index index = system().indexSpeciesWithError(species);
    setSpeciesAmount(index, amount);
}

auto ChemicalState::setSpeciesAmount(Index index, double amount, std::string units) -> void
{
    if(units::convertible(units, "mol"))
        setSpeciesAmount(index, units::convert(amount, units, "mol"));
    else if(units::convertible(units, "kg"))
    {
        const double molar_mass = system().species(index).molarMass();
        setSpeciesAmount(index, units::convert(amount, units, "kg")/molar_mass);
    }
    else errorNonAmountOrMassUnits(units);
}

auto ChemicalState::setSpeciesAmount(std::string species, double amount, std::string units) -> void
{
    const Index index = system().indexSpeciesWithError(species);
    setSpeciesAmount(index, amount, units);
}

auto ChemicalState::setElementPotentials(const Vector& y) -> void
{
    pimpl->y = y;
}

auto ChemicalState::setSpeciesPotentials(const Vector& z) -> void
{
    pimpl->z = z;
}

auto ChemicalState::setVolume(double volume) -> void
{
    Assert(volume >= 0.0, "Cannot set the volume of the chemical state.", "The given volume is negative.");
    const double T = temperature();
    const double P = pressure();
    const Vector& n = speciesAmounts();
    const Vector v = system().phaseVolumes(T, P, n).val;
    const double vtotal = sum(v);
    const double scalar = (vtotal != 0.0) ? volume/vtotal : 0.0;
    scaleSpeciesAmounts(scalar);
}

auto ChemicalState::setPhaseVolume(Index index, double volume) -> void
{
    Assert(volume >= 0.0, "Cannot set the volume of the phase.", "The given volume is negative.");
    Assert(index < system().numPhases(), "Cannot set the volume of the phase.", "The given phase index is out of range.");
    const double T = temperature();
    const double P = pressure();
    const Vector& n = speciesAmounts();
    const Vector v = system().phaseVolumes(T, P, n).val;
    const double scalar = (v[index] != 0.0) ? volume/v[index] : 0.0;
    scaleSpeciesAmountsInPhase(index, scalar);
}

auto ChemicalState::setPhaseVolume(Index index, double volume, std::string units) -> void
{
    volume = units::convert(volume, units, "m3");
    setPhaseVolume(index, volume);
}

auto ChemicalState::setPhaseVolume(std::string name, double volume) -> void
{
    const Index index = system().indexPhase(name);
    setPhaseVolume(index, volume);
}

auto ChemicalState::setPhaseVolume(std::string name, double volume, std::string units) -> void
{
    volume = units::convert(volume, units, "m3");
    setPhaseVolume(name, volume);
}

auto ChemicalState::scaleSpeciesAmounts(double scalar) -> void
{
    Assert(scalar >= 0.0, "Cannot scale the molar amounts of the species.", "The given scalar is negative.");
    for(unsigned i = 0; i < pimpl->n.rows(); ++i)
        pimpl->n[i] *= scalar;
}

auto ChemicalState::scaleSpeciesAmountsInPhase(Index index, double scalar) -> void
{
    Assert(scalar >= 0.0, "Cannot scale the molar amounts of the species.", "The given scalar `" + std::to_string(scalar) << "` is negative.");
    Assert(index < system().numPhases(), "Cannot set the volume of the phase.", "The given phase index is out of range.");
    const Index start = system().indexFirstSpeciesInPhase(index);
    const Index size = system().numSpeciesInPhase(index);
    for(unsigned i = 0; i < size; ++i)
        pimpl->n[start + i] *= scalar;
}

auto ChemicalState::system() const -> const ChemicalSystem&
{
    return pimpl->system;
}

auto ChemicalState::temperature() const -> double
{
    return pimpl->T;
}

auto ChemicalState::pressure() const -> double
{
    return pimpl->P;
}

auto ChemicalState::speciesAmounts() const -> const Vector&
{
    return pimpl->n;
}

auto ChemicalState::elementPotentials() const -> const Vector&
{
    return pimpl->y;
}

auto ChemicalState::speciesPotentials() const -> const Vector&
{
    return pimpl->z;
}

auto ChemicalState::speciesAmount(Index index) const -> double
{
    Assert(index < system().numSpecies(),
        "Cannot get the molar amount of the species.",
        "The given index is out-of-range.");
    return pimpl->n[index];
}

auto ChemicalState::speciesAmount(std::string name) const -> double
{
    return speciesAmount(system().indexSpeciesWithError(name));
}

auto ChemicalState::speciesAmount(Index ispecies, std::string units) const -> double
{
    return units::convert(speciesAmount(ispecies), "mol", units);
}

auto ChemicalState::speciesAmount(std::string species, std::string units) const -> double
{
    return units::convert(speciesAmount(species), "mol", units);
}

auto ChemicalState::elementAmounts() const -> Vector
{
    return system().elementAmounts(speciesAmounts());
}

auto ChemicalState::elementAmountsInPhase(Index iphase) const -> Vector
{
    return system().elementAmountsInPhase(iphase, speciesAmounts());
}

auto ChemicalState::elementAmountsInSpecies(const Indices& ispecies) const -> Vector
{
    return system().elementAmountsInSpecies(ispecies, speciesAmounts());
}

auto ChemicalState::elementAmount(Index ielement) const -> double
{
    return system().elementAmount(ielement, speciesAmounts());
}

auto ChemicalState::elementAmount(std::string element) const -> double
{
    return elementAmount(system().indexElementWithError(element));
}

auto ChemicalState::elementAmount(Index index, std::string units) const -> double
{
    return units::convert(elementAmount(index), "mol", units);
}

auto ChemicalState::elementAmount(std::string name, std::string units) const -> double
{
    return units::convert(elementAmount(name), "mol", units);
}

auto ChemicalState::elementAmountInPhase(Index ielement, Index iphase) const -> double
{
    return system().elementAmountInPhase(ielement, iphase, speciesAmounts());
}

auto ChemicalState::elementAmountInPhase(std::string element, std::string phase) const -> double
{
    const unsigned ielement = system().indexElementWithError(element);
    const unsigned iphase = system().indexPhaseWithError(phase);
    return elementAmountInPhase(ielement, iphase);
}

auto ChemicalState::elementAmountInPhase(Index ielement, Index iphase, std::string units) const -> double
{
    return units::convert(elementAmountInPhase(ielement, iphase), "mol", units);
}

auto ChemicalState::elementAmountInPhase(std::string element, std::string phase, std::string units) const -> double
{
    return units::convert(elementAmountInPhase(element, phase), "mol", units);
}

auto ChemicalState::elementAmountInSpecies(Index ielement, const Indices& ispecies) const -> double
{
    return system().elementAmountInSpecies(ielement, ispecies, speciesAmounts());
}

auto ChemicalState::elementAmountInSpecies(Index ielement, const Indices& ispecies, std::string units) const -> double
{
    return units::convert(elementAmountInSpecies(ielement, ispecies), "mol", units);
}

auto ChemicalState::phaseStabilityIndices() const -> Vector
{
    // Initialize auxiliary variables
    const auto& T = temperature();
    const auto& P = pressure();
    const auto& n = speciesAmounts();
    const auto& x = system().molarFractions(n).val;
    const auto& u = system().chemicalPotentials(T, P, n).val;
    const auto& y = elementPotentials();
    const auto& A = system().formulaMatrix();
    const auto& RT = universalGasConstant * T;

    // Calculate the z-Lagrange multipliers for all species (including kinetic and inert species)
    const Vector z = u - tr(A)*y;

    // Compute an auxiliary vector with the product wi' = xi * exp(-zi/RT)
    const Vector omega = x.array() * exp(-z/RT);

    // Initialise the stability indices of the phases
    Vector stability_indices = zeros(system().numPhases());

    // Iterate over all phases and compute their respective stability indices
    for(unsigned i = 0; i < system().numPhases(); ++i)
    {
        const Index offset = system().indexFirstSpeciesInPhase(i);
        const Index size = system().numSpeciesInPhase(i);
        stability_indices[i] = sum(rows(omega, offset, size));
    }

    // Compute the last step of the stability indices of the equilibrium phases
    stability_indices = log(stability_indices)/std::log(10);

    return stability_indices;
}

auto operator<<(std::ostream& out, const ChemicalState& state) -> std::ostream&
{
    const auto system = state.system();
    const auto T = state.temperature();
    const auto P = state.pressure();
    const auto n = state.speciesAmounts();
    const auto x = system.molarFractions(n).val;
    const auto g = system.activityCoefficients(T, P, n).val;
    const auto a = system.activities(T, P, n).val;
    const auto u = system.chemicalPotentials(T, P, n).val;
    const auto nt_phases = system.phaseMolarAmounts(n).val;
    const auto mt_phases = system.phaseMassAmounts(n);
    const auto v_phases  = system.phaseMolarVolumes(T, P, n).val;
    const auto vt_phases = system.phaseVolumes(T, P, n).val;
    const auto vf_phases = vt_phases/sum(vt_phases);
    const auto stability_phases = state.phaseStabilityIndices();

    // Calculate pH, pe, and Eh
//    const double F = 96485.3365; // the Faraday constant
//    const double pH = state.acidity(state.activities());
//    const double pe = yc.rows() ? - yc[0]/RT/std::log(10) : 0.0;
//    const double Eh = yc.rows() ? - yc[0]/F : 0.0;
//
//    // Calculate the ionic strength
//    const double I = 0.5 * (state.composition().array() * system.speciesCharges().array().pow(2)).sum();

    const unsigned num_phases = system.numPhases();
    const unsigned bar_size = std::max(unsigned(8), num_phases + 2) * 25;
    const std::string bar1(bar_size, '=');
    const std::string bar2(bar_size, '-');

    out << bar1 << std::endl;
    out << std::setw(25) << std::left << "Temperature [K]";
    out << std::setw(25) << std::left << "Temperature [°C]";
    out << std::setw(25) << std::left << "Pressure [MPa]";
    out << std::endl << bar2 << std::endl;

    out << std::setw(25) << std::left << T;
    out << std::setw(25) << std::left << T - 273.15;
    out << std::setw(25) << std::left << P * 1e-6;
    out << std::endl;

    // Set output in scientific notation
    auto flags = out.flags();
    out << std::scientific << std::setprecision(6);

    // Output the table of the element-related state
    out << bar1 << std::endl;
    out << std::setw(25) << std::left << "Element";
    out << std::setw(25) << std::left << "Amount [mol]";
    for(const auto& phase : system.phases())
        out << std::setw(25) << std::left << phase.name() + " [mol]";
    out << std::endl;
    out << bar2 << std::endl;
    for(unsigned i = 0; i < system.numElements(); ++i)
    {
        out << std::setw(25) << std::left << system.element(i).name();
        out << std::setw(25) << std::left << system.elementAmount(i, n);
        for(unsigned j = 0; j < system.numPhases(); ++j)
            out << std::setw(25) << std::left << system.elementAmountInPhase(i, j, n);
        out << std::endl;
    }

    // Output the table of the species-related state
    out << bar1 << std::endl;
    out << std::setw(25) << std::left << "Species";
    out << std::setw(25) << std::left << "Amount [mol]";
    out << std::setw(25) << std::left << "MolarFraction [mol/mol]";
    out << std::setw(25) << std::left << "ActivityCoefficient [-]";
    out << std::setw(25) << std::left << "Activity [-]";
    out << std::setw(25) << std::left << "ChemicalPotential [kJ/mol]";
    out << std::endl;
    out << bar2 << std::endl;
    for(unsigned i = 0; i < system.numSpecies(); ++i)
    {
        out << std::setw(25) << std::left << system.species(i).name();
        out << std::setw(25) << std::left << n[i];
        out << std::setw(25) << std::left << x[i];
        out << std::setw(25) << std::left << g[i];
        out << std::setw(25) << std::left << a[i];
        out << std::setw(25) << std::left << u[i]/1000; // convert from J/mol to kJ/mol
        out << std::endl;
    }

    // Output the table of the phase-related state
    out << bar1 << std::endl;
    out << std::setw(25) << std::left << "Phase";
    out << std::setw(25) << std::left << "Amount [mol]";
    out << std::setw(25) << std::left << "StabilityIndex [-]";
    out << std::setw(25) << std::left << "Mass [kg]";
    out << std::setw(25) << std::left << "Volume [m3]";
    out << std::setw(25) << std::left << "VolumeFraction [m3/m3]";
    out << std::setw(25) << std::left << "Density [m3/kg]";
    out << std::setw(25) << std::left << "MolarVolume [m3/mol]";
    out << std::endl;
    out << bar2 << std::endl;
    for(unsigned i = 0; i < system.numPhases(); ++i)
    {
        int extra = (stability_phases[i] < 0 ? 0 : 1);
        out << std::setw(25) << std::left << system.phase(i).name();
        out << std::setw(25 + extra) << std::left << nt_phases[i];
        out << std::setw(25 - extra) << std::left << stability_phases[i];
        out << std::setw(25) << std::left << mt_phases[i];
        out << std::setw(25) << std::left << vt_phases[i];
        out << std::setw(25) << std::left << vf_phases[i];
        out << std::setw(25) << std::left << vt_phases[i]/mt_phases[i];
        out << std::setw(25) << std::left << v_phases[i];
        out << std::endl;
    }

    out << bar1 << std::endl;

    // Recover the previous state of `out`
    out.flags(flags);

    return out;
}

auto operator+(const ChemicalState& l, const ChemicalState& r) -> ChemicalState
{
    const Vector& nl = l.speciesAmounts();
    const Vector& nr = r.speciesAmounts();
    ChemicalState res = l;
    res.setSpeciesAmounts(nl + nr);
    return res;
}

auto operator*(double scalar, const ChemicalState& state) -> ChemicalState
{
    ChemicalState res = state;
    res.scaleSpeciesAmounts(scalar);
    return res;
}

auto operator*(const ChemicalState& state, double scalar) -> ChemicalState
{
    return scalar*state;
}

} // namespace Reaktoro
