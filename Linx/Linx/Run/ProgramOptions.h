// @copyright 2022-2024, Antoine Basset (CNES)
// This file is part of Linx <github.com/kabasset/Linx>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef _LINXRUN_PROGRAMOPTIONS_H
#define _LINXRUN_PROGRAMOPTIONS_H

#include <boost/program_options.hpp>
#include <iostream>
#include <sstream>

namespace Linx {

/**
 * @brief Helper class to declare positional, named and flag options, as well as some help message.
 * 
 * Here is an example use case for the following command line:
 * \verbatim Program <positional> --named1 <value1> -f --named2 <value2> \endverbatim
 * 
 * Here's an example program to handle it:
 * \code
 * std::pair<OptionsDescription, PositionalOptionsDescription> defineProgramArguments() override
 * {
 *   ProgramOptions options("My program");
 *   options.positional<std::string>("positional", "Positional option");
 *   options.named<int>("named1", "Named option 1");
 *   options.named<int>("named2", "Named option 2");
 *   options.flag("flag,f", "Flag");
 *   return options.as_pair();
 * }
 * \endcode
 */
class ProgramOptions {
public:

  /**
   * @brief Shortcut for Boost.ProgramOptions type.
   */
  using OptionsDescription = boost::program_options::options_description;

  /**
   * @brief Shortcut for Boost.ProgramOptions type.
   */
  using PositionalOptionsDescription = boost::program_options::positional_options_description;

  /**
   * @brief Shortcut for Boost.ProgramOptions type.
   */
  using ValueSemantics = boost::program_options::value_semantic;

  /**
   * @brief Make a `ProgramOptions` with optional description string and help option.
   * @param description The program description
   * @param help The help option (disapled if parameter is empty)
   */
  ProgramOptions(const std::string& description = "", const std::string& help = "help,h") :
      m_named("Options", 120), m_add(m_named.add_options()), m_positional(), m_variables(), m_desc(description),
      m_help(help)
  {
    if (help.length() > 0) {
      flag(m_help.c_str(), "Print help message");
      if (m_help.length() > 3 && m_help[m_help.length() - 2] == ',') {
        m_help = m_help.substr(0, m_help.length() - 2);
      }
    }
  }

  /**
   * @brief Declare a positional option.
   */
  template <typename T>
  void positional(const char* name, const char* description)
  {
    positional(name, boost::program_options::value<T>(), description);
  }

  /**
   * @brief Declare a positional option with default value.
   */
  template <typename T>
  void positional(const char* name, const char* description, T default_value)
  {
    positional(name, boost::program_options::value<T>()->default_value(default_value), description);
  }

  /**
   * @brief Declare a positional option with custom semantics.
   */
  void positional(const char* name, const ValueSemantics* value, const char* description)
  {
    m_add(name, value, description);
    const int max_args = value->max_tokens();
    m_positional.add(name, max_args);
  }

  /**
   * @brief Declare a named option.
   * 
   * A short form (1-character) of the option can be provided, separated by a comma.
   */
  template <typename T>
  void named(const char* name, const char* description)
  {
    named(name, boost::program_options::value<T>(), description);
  }

  /**
   * @brief Declare a named option with default value.
   * 
   * A short form (1-character) of the option can be provided, separated by a comma.
   */
  template <typename T>
  void named(const char* name, const char* description, T default_value)
  {
    named(name, boost::program_options::value<T>()->default_value(default_value), description);
  }

  /**
   * @brief Declare a named option with custom semantics.
   * 
   * A short form (1-character) of the option can be provided, separated by a comma.
   */
  void named(const char* name, const ValueSemantics* value, const char* description)
  {
    m_add(name, value, description);
  }

  /**
   * @brief Declare a flag option.
   */
  void flag(const char* name, const char* description)
  {
    named(name, boost::program_options::value<bool>()->default_value(false)->implicit_value(true), description);
  }

  /**
   * @brief Get the named (flags included) and positional options as a pair.
   */
  std::pair<OptionsDescription, PositionalOptionsDescription> as_pair() const
  {
    return std::make_pair(m_named, m_positional);
  }

  /**
   * @brief Parse a command line.
   * 
   * If the help option was enabled and is in the command line, then the help message is printed and the program stops.
   */
  void parse(int argc, const char* const argv[])
  {
    boost::program_options::store(
        boost::program_options::command_line_parser(argc, argv).options(m_named).positional(m_positional).run(),
        m_variables);
    boost::program_options::notify(m_variables);
    if (not m_help.empty() && as<bool>(m_help.c_str())) {
      if (not m_desc.empty()) {
        std::cout << "\n" << m_desc << "\n";
      }
      std::cout << "\nUsage:\n  " << argv[0] << " [options]\n\n" << m_named << std::endl;
      exit(0);
    }
  }

  /**
   * @brief Parse a command line (vector of `const char*` arguments).
   */
  void parse(const std::vector<const char*>& args)
  {
    parse(args.size(), args.data());
  }

  /**
   * @brief Parse a command line (vector of string arguments).
   */
  void parse(const std::vector<std::string>& args)
  {
    std::vector<const char*> cstr(args.size());
    std::transform(args.begin(), args.end(), cstr.begin(), [](const auto& a) {
      return a.c_str();
    });
    parse(cstr);
  }

  /**
   * @brief Parse a command line (space-separated arguments as a single string).
   */
  void parse(const std::string& args)
  {
    std::istringstream iss(args);
    parse(std::vector<std::string>(std::istream_iterator<std::string> {iss}, std::istream_iterator<std::string>()));
  }

  /**
   * @brief Check whether a given option is set.
   */
  bool has(const char* name) const
  {
    return m_variables.count(name); // FIXME incompatible with flags
  }

  /**
   * @brief Get the value of a given option.
   * 
   * Throws if the option is not set.
   */
  template <typename T>
  T as(const char* name) const
  {
    return m_variables[name].as<T>();
  }

private:

  OptionsDescription m_named;
  boost::program_options::options_description_easy_init m_add;
  PositionalOptionsDescription m_positional;
  boost::program_options::variables_map m_variables;
  std::string m_desc;
  std::string m_help;
};

} // namespace Linx

#endif
