/*
 * Copyright (C) 2014-2016 Olzhas Rakhimov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// @file xml_parser.h
/// XML Parser.

#ifndef SCRAM_SRC_XML_PARSER_H_
#define SCRAM_SRC_XML_PARSER_H_

#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <libxml++/libxml++.h>

#include "error.h"

namespace scram {

/// Initializes a DOM parser
/// and converts library exceptions into local errors.
///
/// @param[in] xml_input_snippet  An XML snippet to be used as input.
///
/// @returns A parser with a well-formed, initialized document.
///
/// @throws ValidationError  There are problems loading the XML snippet.
inline std::unique_ptr<xmlpp::DomParser> ConstructDomParser(
    const std::string& file_path) {
  try {
    return std::make_unique<xmlpp::DomParser>(file_path);
  } catch (const xmlpp::parse_error& ex) {
    throw ValidationError("XML file is invalid: " + std::string(ex.what()));
  }
}

/// Validates the file against a schema.
///
/// @param[in] document  Well-formed, initialized document.
/// @param[in] xml_schema_snippet  The schema to validate against.
///
/// @throws ValidationError  The XML file failed schema validation.
/// @throws LogicError  The schema could not be parsed.
void Validate(const xmlpp::Document* document,
              const std::stringstream& xml_schema_snippet);

/// Helper function to statically cast to XML element.
///
/// @param[in] node  XML node known to be XML element.
///
/// @returns XML element cast from the XML node.
///
/// @warning The node must be an XML element.
inline const xmlpp::Element* XmlElement(const xmlpp::Node* node) {
  return static_cast<const xmlpp::Element*>(node);
}

/// Normalizes the string in an XML attribute.
///
/// @param[in] element  XML element with the attribute.
/// @param[in] attribute  The name of the attribute.
///
/// @returns Normalized (trimmed) string from the attribute.
inline std::string GetAttributeValue(const xmlpp::Element* element,
                                     const std::string& attribute) {
  std::string value = element->get_attribute_value(attribute);
  boost::trim(value);
  return value;
}

/// Gets a number from an XML attribute.
///
/// @tparam T  Numerical type.
///
/// @param[in] element  XML element with the attribute.
/// @param[in] attribute  The name of the attribute.
///
/// @returns The interpreted value.
///
/// @throws ValidationError  Casting is unsuccessful.
///                          The error message will include the line number.
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
CastAttributeValue(const xmlpp::Element* element,
                   const std::string& attribute) {
  try {
    return boost::lexical_cast<T>(GetAttributeValue(element, attribute));
  } catch (boost::bad_lexical_cast&) {
    std::stringstream msg;
    msg << "Line " << element->get_line() << ":\n"
        << "Failed to interpret attribute '" << attribute << "' to a number.";
    throw ValidationError(msg.str());
  }
}

/// Gets a number from an XML text.
///
/// @tparam T  Numerical type.
///
/// @param[in] element  XML element with the text.
///
/// @returns The interpreted value.
///
/// @throws ValidationError  Casting is unsuccessful.
///                          The error message will include the line number.
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
CastChildText(const xmlpp::Element* element) {
  std::string content = element->get_child_text()->get_content();
  boost::trim(content);
  try {
    return boost::lexical_cast<T>(content);
  } catch (boost::bad_lexical_cast&) {
    std::stringstream msg;
    msg << "Line " << element->get_line() << ":\n"
        << "Failed to interpret text '" << content << "' to a number.";
    throw ValidationError(msg.str());
  }
}

}  // namespace scram

#endif  // SCRAM_SRC_XML_PARSER_H_
