// Reaktoro is a unified framework for modeling chemically reactive systems.
//
// Copyright © 2014-2022 Allan Leal
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.

#include "Core.Data.hpp"

// Reaktoro includes
#include <Reaktoro/Core/ChemicalFormula.hpp>
#include <Reaktoro/Core/ChemicalSystem.hpp>
#include <Reaktoro/Core/Database.hpp>
#include <Reaktoro/Core/Element.hpp>
#include <Reaktoro/Core/ElementList.hpp>
#include <Reaktoro/Core/FormationReaction.hpp>
#include <Reaktoro/Core/Param.hpp>
#include <Reaktoro/Core/Phase.hpp>
#include <Reaktoro/Core/Species.hpp>
#include <Reaktoro/Core/SpeciesList.hpp>
#include <Reaktoro/Core/Support/DatabaseParserYAML.hpp>
#include <Reaktoro/Models/StandardThermoModels/ReactionStandardThermoModelYAML.hpp>
#include <Reaktoro/Models/StandardThermoModels/StandardThermoModelYAML.hpp>
#include <Reaktoro/Serialization/Common.Data.hpp>

namespace Reaktoro {

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(ChemicalFormula)
{
    data = obj.str();
}

REAKTORO_DATA_DECODE_DEFINE(ChemicalFormula)
{
    obj = ChemicalFormula(data.asString());
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(ChemicalSystem)
{
    errorif(true, "Implement REAKTORO_DATA_ENCODE_DEFINE(ChemicalSystem)");
}

REAKTORO_DATA_DECODE_DEFINE(ChemicalSystem)
{
    errorif(true, "Implement REAKTORO_DATA_DECODE_DEFINE(ChemicalSystem)");
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(Database)
{
    auto elements_data = data.at("Elements");
    auto species_data = data.at("Species");

    for(auto const& element : obj.elements())
        elements_data.at(element.symbol()) = element;

    for(auto const& species : obj.species())
        species_data.at(species.name()) = species;
}

REAKTORO_DATA_DECODE_DEFINE(Database)
{
    // obj = DatabaseParserYAML(data);
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(Element)
{
    data.at("Symbol")    = obj.symbol();
    data.at("MolarMass") = obj.molarMass();
    if(obj.name().size()) data.at("Name") = obj.name();
    if(obj.tags().size()) data.at("Tags") = obj.tags();
}

REAKTORO_DATA_DECODE_DEFINE(Element)
{
    Element::Attribs attribs;
    data.at("Symbol").to(attribs.symbol);
    data.at("MolarMass").to(attribs.molar_mass);
    if(data.exists("Name")) data.at("Name").to(attribs.name);
    if(data.exists("Tags")) data.at("Tags").to(attribs.tags);
    obj = Element(attribs);
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(ElementList)
{
    for(auto const& element : obj)
        data.add(element);
}

REAKTORO_DATA_DECODE_DEFINE(ElementList)
{
    obj = data.as<Vec<Element>>();
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(ElementalComposition)
{
    data = obj.repr();
}

REAKTORO_DATA_DECODE_DEFINE(ElementalComposition)
{
    errorif(true, "Converting YAML to ElementalComposition is not supported directly."); // because only element symbols are present in YAML representation of ElementalComposition
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(FormationReaction)
{
    std::stringstream repr;
    auto i = 0;
    for(const auto& [species, coeff] : obj.reactants())
        repr << (i++ == 0 ? "" : " ") << coeff << ":" << species.name();

    data.at("Reactants") = repr.str();
    if(obj.reactionThermoModel().initialized())
        data.at("ReactionStandardThermoModel") = obj.reactionThermoModel().serialize();
    if(obj.productStandardVolumeModel().initialized())
        data.at("StandardVolumeModel") = obj.productStandardVolumeModel().serialize();
}

REAKTORO_DATA_DECODE_DEFINE(FormationReaction)
{
    errorif(true, "Converting YAML to FormationReaction is not supported directly."); // because only species names are present in YAML representation of Reactants
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(Phase)
{
    errorif(true, "Implement REAKTORO_DATA_ENCODE_DEFINE(Phase)");
}

REAKTORO_DATA_DECODE_DEFINE(Phase)
{
    errorif(true, "Implement REAKTORO_DATA_DECODE_DEFINE(Phase)");
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(ReactionStandardThermoModel)
{
    data = obj.serialize();
}

REAKTORO_DATA_DECODE_DEFINE(ReactionStandardThermoModel)
{
    // obj = ReactionStandardThermoModelYAML(data);
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(Species)
{
    data.at("Name") = obj.name();
    data.at("Formula") = obj.formula();
    data.at("Substance") = obj.substance();
    data.at("Elements") = obj.elements();
    if(obj.charge() != 0.0) data.at("Charge") = obj.charge();
    data.at("AggregateState") = obj.aggregateState();
    if(obj.reaction().reactants().size())
        data.at("FormationReaction") = obj.reaction();
    else data.at("StandardThermoModel") = obj.standardThermoModel();
    if(obj.tags().size()) data.at("Tags") = obj.tags();
}

REAKTORO_DATA_DECODE_DEFINE(Species)
{
    errorif(true, "Converting YAML to Species is not supported directly."); // because only element symbols are present in YAML representation of Species
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(SpeciesList)
{
    for(auto const& species : obj)
        data.add(species);
}

REAKTORO_DATA_DECODE_DEFINE(SpeciesList)
{
    errorif(true, "Converting YAML to SpeciesList is not supported directly."); // because only element symbols are present in YAML representation of Species
}

//=====================================================================================================================

REAKTORO_DATA_ENCODE_DEFINE(StandardThermoModel)
{
    data = obj.serialize();
}

REAKTORO_DATA_DECODE_DEFINE(StandardThermoModel)
{
    // obj = StandardThermoModelYAML(data);
}

//=====================================================================================================================

} // namespace YAML