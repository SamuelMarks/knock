#include <iostream>
#include <filesystem>

#include "drmprocessorclientimpl.h"
#include "libgourou.h"
#include "libgourou_common.h"

#ifndef KNOCK_VERSION
#error KNOCK_VERSION must be defined
#endif /* !KNOCK_VERSION */

std::string get_data_dir();
void verify_absence(const std::string &);
void verify_presence(const std::string &);

int main(int argc, char **argv) try {

  if (argc == 1) {
    std::cout << "info: knock version " << KNOCK_VERSION
              << ", libgourou version " << LIBGOUROU_VERSION << "\n"
              << "usage: " << argv[0] << " [ACSM]\n"
              << "result: converts file ACSM to a plain EPUB/PDF if present, "
                 "otherwise prints this"
              << std::endl;
    return EXIT_SUCCESS;
  }

  if (argc != 2) {
    throw std::invalid_argument("the ACSM file must be passed as an argument");
  }

  const std::string acsm_file = argv[1];
  verify_presence(acsm_file);
  const std::string acsm_stem =
      acsm_file.substr(0, acsm_file.find_last_of('.'));
  const std::string drm_file = acsm_stem + ".drm";
  const std::string out_file = acsm_stem + ".out";
  verify_absence(drm_file);
  verify_absence(out_file);
  const std::string knock_data = get_data_dir();

  DRMProcessorClientImpl client;
  gourou::DRMProcessor *processor = gourou::DRMProcessor::createDRMProcessor(
      &client,
      false, // don't "always generate a new device" (default)
      knock_data);

  std::cout << "anonymously signing in..." << std::endl;
  processor->signIn("anonymous", "");
  processor->activateDevice();

  std::cout << "downloading the file from Adobe..." << std::endl;
  gourou::FulfillmentItem *item = processor->fulfill(acsm_file);
  gourou::DRMProcessor::ITEM_TYPE type = processor->download(item, drm_file);

  std::cout << "removing DRM from the file..." << std::endl;
  std::string ext_file;
  std::string file_type;
  switch (type) {
  case gourou::DRMProcessor::ITEM_TYPE::PDF: {
    // for pdfs the function moves the pdf while removing drm
    processor->removeDRM(drm_file, out_file, type, nullptr, 0);
    std::filesystem::remove(drm_file);
    ext_file = acsm_stem + ".pdf";
    file_type = "PDF";
    break;
  }
  case gourou::DRMProcessor::ITEM_TYPE::EPUB: {
    // for epubs the drm is removed in-place so in == out
    processor->removeDRM(drm_file, drm_file, type, nullptr, 0);
    std::filesystem::rename(drm_file, out_file);
    ext_file = acsm_stem + ".epub";
    file_type = "EPUB";
    break;
  }
  }

  if (std::filesystem::exists(ext_file)) {
    std::cerr << "warning: failed to update file extension; " + ext_file +
                     " already exists"
              << std::endl;
    ext_file = out_file;
  } else {
    std::filesystem::rename(out_file, ext_file);
  }

  std::filesystem::remove(acsm_file);
  std::cout << file_type + " file generated at " + ext_file << std::endl;

  return EXIT_SUCCESS;

} catch (const gourou::Exception &e) {
  std::cerr << "error:\n" << e.what();
  return EXIT_FAILURE;
} catch (const std::exception &e) {
  std::cerr << "error: " << e.what() << std::endl;
  return EXIT_FAILURE;
}

std::string get_data_dir() {
  const char *xdg_data_home = std::getenv("XDG_DATA_HOME");
  std::filesystem::path knock_data;
  if (xdg_data_home != nullptr) {
    knock_data = xdg_data_home;
  } else {
    knock_data = std::filesystem::path(std::getenv("HOME")) / ".local" / "share";
  }
  knock_data /= "knock" / std::filesystem::path("acsm");
  return knock_data;
}

void verify_absence(const std::string &file) {
  if (std::filesystem::exists(file)) {
    throw std::runtime_error("file " + file +
                             " must be moved out of the way or deleted");
  }
}

void verify_presence(const std::string &file) {
  if (!std::filesystem::exists(file)) {
    throw std::runtime_error("file " + file + " does not exist");
  }
}
