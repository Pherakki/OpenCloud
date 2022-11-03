#include <cstddef>
#include <vector>

#include <Windows.h>
#include <shellapi.h>

#include "common/types.h"
#include "common/strings.h"
#include "common/console.h"
#include "common/log.h"
#include "common/console_logger.h"
#include "common/file_helpers.h"

#include "iso9660/extractor.h"

set_log_channel("ExtractISO")

using cmd_args = std::tuple<bool, std::optional<std::string>, std::optional<std::string>>;

using namespace common;

static std::vector<std::string> get_cmd_line_utf8()
{
  // we're using this instead of the lpCmdLine passed to winmain
  // the version passed to winmain does not include the exe name
  const LPWSTR lpCmdLine = GetCommandLineW();

  int count;
  LPWSTR* szArglist = CommandLineToArgvW(lpCmdLine, &count);

  std::vector<std::string> utf8_args;

  // convert everything to UTF-8
  for (int i = 0; i < count; ++i)
  {
    auto arg = common::strings::to_utf8(szArglist[i]);

    if (arg)
      utf8_args.push_back(std::move(*arg));
  }

  // > the calling application must free the memory used by the argument list when it is no longer needed.
  LocalFree(szArglist);

  return utf8_args;
}

static cmd_args parse_args(std::vector<std::string>& arg_list)
{
  std::optional<std::string> iso_path;
  std::optional<std::string> output_dir;

  bool help = false;

  for (auto i = 0; i < arg_list.size(); ++i)
  {
    // print help
    if (arg_list[i] == "-h" || arg_list[i] == "--help")
      help = true;

    // set the iso path (required)
    if (arg_list[i] == "-i" || arg_list[i] == "--iso")
    {
      if (i + 1 < arg_list.size())
        iso_path = arg_list[++i];
    }

    // set the output directory (not required)
    if (arg_list[i] == "-o" || arg_list[i] == "--output")
    {
      if (i + 1 < arg_list.size())
        output_dir = arg_list[++i];
    }
  }

  return { help, iso_path, output_dir };
}

static void print_help(std::string_view prog_name)
{
  console::writeln_format("{}: [OPTION]...", prog_name);
  console::writeln("Extracts the contents of a Dark Cloud 2 ISO");
  console::writeln("\t--iso -i [path]\t\tSpecify an iso file to open");
  console::writeln("\t--help -h\t\tPrint this help message");
}

static bool initialize_console_logger()
{
  if (!console::initialize())
    return false;

  if (!log::console_logger::initialize())
    return false;

  return true;
}

static void shutdown_console_logger()
{
  log::console_logger::shutdown();
  console::shutdown();
}

INT WINAPI WinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/,
  _In_ LPSTR /*lpCmdLine*/, _In_ INT /*nCmdShow*/)

{
  // we need the console for this
  if (!initialize_console_logger())
    return EXIT_FAILURE;

  auto arg_list = get_cmd_line_utf8();

  // set the application directory to the parent of this app
  file_helpers::set_application_directory(file_helpers::parent_directory(arg_list.front()));

  // get the program name as the basename of the app name
  std::string_view program_name = file_helpers::basename(arg_list.front());

  auto [help, iso_path, output] = parse_args(arg_list);

  if (help)
  {
    print_help(program_name);

    shutdown_console_logger();

    return EXIT_SUCCESS;
  }

  if (!iso_path)
  {
    log_error("ISO path required.");

    print_help(program_name);

    shutdown_console_logger();

    return EXIT_FAILURE;
  }

  auto iso = iso9660::extractor::open(*iso_path);

  if (!iso)
  {
    log_error("Unable to open ISO {}.", *iso_path);

    shutdown_console_logger();

    return EXIT_FAILURE;
  }

  // default to the working directory
  auto output_directory = output.value_or(file_helpers::append(file_helpers::get_application_directory(), "DATA"));

  if (!file_helpers::create_directory(output_directory))
  {
    log_error("Unable to create directory {}", output_directory);

    shutdown_console_logger();

    return EXIT_FAILURE;
  }

  // these are the files we're interested in
  static constexpr std::array files =
  {
    "DATA.DAT",
    "DATA.HD2",
    "DATA.HD3",
    "SOUND.DAT",
    "SOUND.HD3"
  };

  static std::array<std::string, 3> directories =
  {
    "MOVIE",
    common::file_helpers::append("MOVIE", "TUTO"),
    common::file_helpers::append("MOVIE", "TUTO2")
  };

  for (const auto& dir : directories)
  {
    const auto full_path = file_helpers::append(output_directory, dir);

    log_info("Creating directory {}", full_path);

    if (!file_helpers::create_directory(full_path))
    {
      log_error("Unable to create directory {}", full_path);

      shutdown_console_logger();

      return EXIT_FAILURE;
    }
  }

  for (const auto& filename : files)
  {
    if (!iso->extract_file(filename, output_directory))
    {
      log_error("Failed to extract file {}", filename);

      shutdown_console_logger();

      return EXIT_FAILURE;
    }
  }

  for (const auto& dir : directories)
  {
    if (!iso->extract_directory(dir, output_directory))
    {
      log_error("Failed to extract directory: {}", dir);

      shutdown_console_logger();

      return EXIT_FAILURE;
    }
  }

  shutdown_console_logger();

  return EXIT_SUCCESS;
}
