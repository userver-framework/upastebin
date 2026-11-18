#include "text.hpp"

#include <userver/components/component_context.hpp>
#include <userver/components/fs_cache.hpp>
#include <userver/utils/overloaded.hpp>

namespace upastebin {

TextHandler::TextHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      fs_client_(
          context.FindComponent<userver::components::FsCache>("resources-cache")
              .GetClient()) {}

std::string TextHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto id = request.GetPathArg("id");

  auto file_ptr = fs_client_.TryGetFile("/text.html");
  UINVARIANT(file_ptr, "text.html is not found");

  auto& response = request.GetHttpResponse();
  response.SetContentType("text/html; charset=UTF-8");
  return userver::utils::Visit(
      file_ptr->data_or_path,
      [](const std::string& data) { return data; },
      [&response](const auto& path) {
          response.SetStatus(userver::server::http::HttpStatus::kPayloadTooLarge);
          return std::string{};
      }
  );
}

}  // namespace upastebin
