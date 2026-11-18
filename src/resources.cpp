#include "resources.hpp"

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/components/fs_cache.hpp>
#include <userver/utils/overloaded.hpp>

namespace upastebin {

namespace {

std::string_view GetContentType(std::string_view extention) {
  if (extention == ".js")
    return "application/javascript";
  else if (extention == ".css")
    return "text/css";
  else if (extention == ".html")
    return "text/html; charset=UTF-8";
  else
    return "application/octet-stream";
}

}  // namespace

ResourcesHandler::ResourcesHandler(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context),
      fs_client_(
          context.FindComponent<userver::components::FsCache>("resources-cache")
              .GetClient()) {}

std::string ResourcesHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext&) const {
  auto subpath = request.GetPathArg("subpath");
  // Note: no need to validate subpath as invalid name would lead to cache miss
  // in FsCacheClient

  auto file_ptr = fs_client_.TryGetFile("/" + subpath);
  auto& response = request.GetHttpResponse();
  if (file_ptr) {
    return userver::utils::Visit(
        file_ptr->data_or_path,
        [&](const std::string& data) {
            response.SetContentType(GetContentType(file_ptr->extension));
            response.SetHeader(USERVER_NAMESPACE::http::headers::kExpires, "600");
            return data;
        },
        [&](const auto& path) {
            response.SetStatus(userver::server::http::HttpStatus::kPayloadTooLarge);
            return std::string{};
        }
    );
  } else {
    response.SetStatus(userver::server::http::HttpStatus::kNotFound);
    return {};
  }
}

}  // namespace upastebin
