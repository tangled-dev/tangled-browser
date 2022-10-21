#ifndef CHROME_BROWSER_UI_WEBUI_MILLIX_WS_UI_H_
#define CHROME_BROWSER_UI_WEBUI_MILLIX_WS_UI_H_
#pragma once

#include "ui/webui/untrusted_web_ui_controller.h"

namespace content {
  class WebUI;
  class WebContents;
}

// The WebUI for chrome-untrusted://millix-ws
class UntrustedMillixWSUI : public ui::UntrustedWebUIController {
 public:
  explicit UntrustedMillixWSUI(content::WebUI* web_ui);
  ~UntrustedMillixWSUI() override = default;
};

#endif  // CHROME_BROWSER_UI_WEBUI_MILLIX_WS_UI_H_