#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_WS_CONFIG_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_WS_CONFIG_H_
#pragma once

#include "content/public/browser/webui_config.h"

class UntrustedMillixWSConfig : public content::WebUIConfig {
 public:
  explicit UntrustedMillixWSConfig();
  ~UntrustedMillixWSConfig() override = default;

  std::unique_ptr<content::WebUIController> CreateWebUIController(
      content::WebUI* web_ui) override;
};

#endif // CHROME_BROWSER_UI_WEBUI_MILLIX_WS_CONFIG_H_