#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_
#pragma once

#include "ui/webui/untrusted_web_ui_controller.h"

namespace content {
  class WebUI;
  class WebContents;
}

// The WebUI for chrome-untrusted://millix
class UntrustedMillixAppUI : public ui::UntrustedWebUIController {
 public:
  explicit UntrustedMillixAppUI(content::WebUI* web_ui);
  ~UntrustedMillixAppUI() override = default;
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_APP_UI_H_