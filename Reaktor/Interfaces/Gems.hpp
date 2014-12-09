// Reaktor is a C++ library for computational reaction modelling.
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

#pragma once

// C++ includes
#include <memory>

// Reaktor includes
#include <Reaktor/Common/Matrix.hpp>
#include <Reaktor/Common/Vector.hpp>

// Forward declarations
class TNode;

namespace Reaktor {

/// A wrapper class for Gems code
class Gems
{
public:
    /// Construct a default Gems instance
    Gems();

    /// Construct a Gems instance from a specification file
    /// @param filename The name of the file containing the definition of the chemical system
    Gems(std::string filename);

    /// Set the temperature of the Gems instance (298.15 K by default) (in units of K)
    auto setTemperature(double val) -> void;

    /// Set the pressure of the Gems instance (10<sup>5</sup> Pa by default) (in units of Pa)
    auto setPressure(double val) -> void;

    /// Set the amounts of the species of the Gems instance (in units of mol)
    auto setSpeciesAmounts(const Vector& n) -> void;

    /// Set the amounts of the components of the Gems instance (in units of mol)
    auto setComponentAmounts(const Vector& b) -> void;

    /// Get the number of components
    auto numComponents() const -> unsigned;

    /// Get the number of species
    auto numSpecies() const -> unsigned;

    /// Get the number of phases
    auto numPhases() const -> unsigned;

    /// Get the number of species in a phase
    /// @param index The index of the phase
    auto numSpeciesInPhase(unsigned index) const -> unsigned;

    /// Get the name of a component
    /// @param index The index of the component
    auto componentName(unsigned index) const -> std::string;

    /// Get the name of a species
    /// @param index The index of the species
    auto speciesName(unsigned index) const -> std::string;

    /// Get the name of a phase
    /// @param index The index of the phase
    auto phaseName(unsigned index) const -> std::string;

    /// Get the index of a component
    /// @param name The name of the component
    auto componentIndex(std::string name) const -> unsigned;

    /// Get the index of a species
    /// @param name The name of the species
    auto speciesIndex(std::string name) const -> unsigned;

    /// Get the index of a phase
    /// @param name The name of the phase
    auto phaseIndex(std::string name) const -> unsigned;

    /// Get the molar mass of a component (in units of kg/mol)
    /// @param index The index of the component
    auto componentMolarMass(unsigned index) const -> double;

    /// Get the molar mass of a species (in units of kg/mol)
    /// @param index The index of the species
    auto speciesMolarMass(unsigned index) const -> double;

    /// Get the temperature of the Gems instance (in units of K)
    auto temperature() const -> double;

    /// Get the pressure of the Gems instance (in units of Pa)
    auto pressure() const -> double;

    /// Get the amounts of the components (in units of mol)
    auto componentAmounts() const -> Vector;

    /// Get the amounts of the species (in units of mol)
    auto speciesAmounts() const -> Vector;

    /// Get the balance matrix of the species
    auto balanceMatrix() const -> Matrix;

    /// Get the molar standard Gibbs free energies of the species
    auto gibbsEnergies() -> Vector;

    /// Get the chemical potentials of the species
    auto chemicalPotentials() -> Vector;

    /// Calculate the equilibrium state of the system
    auto equilibrate() -> void;

    /// Get the convergence result of the equilibrium calculation
    auto converged() const -> bool;

    /// Get the number of iterations of the equilibrium calculation
    auto numIterations() const -> unsigned;

    /// Get the wall time of the equilibrium calculation (in units of s)
    auto wallTime() const -> double;

    /// Get a reference to the TNode instance of Gems
    auto node() -> TNode&;

    /// Get a const reference to the TNode instance of Gems
    auto node() const -> const TNode&;

private:
    struct Impl;

    std::unique_ptr<Impl> pimpl;
};

} // namespace Reaktor
