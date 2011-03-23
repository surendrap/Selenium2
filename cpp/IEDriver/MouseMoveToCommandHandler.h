#ifndef WEBDRIVER_IE_MOUSEMOVETOCOMMANDHANDLER_H_
#define WEBDRIVER_IE_MOUSEMOVETOCOMMANDHANDLER_H_

#include "interactions.h"
#include "Session.h"

namespace webdriver {

class MouseMoveToCommandHandler : public WebDriverCommandHandler {
public:
	MouseMoveToCommandHandler(void) {
	}

	virtual ~MouseMoveToCommandHandler(void) {
	}

protected:
	void MouseMoveToCommandHandler::ExecuteInternal(const Session& session, const LocatorMap& locator_parameters, const ParametersMap& command_parameters, WebDriverResponse * response) {
		ParametersMap::const_iterator element_parameter_iterator = command_parameters.find("element");
		ParametersMap::const_iterator xoffset_parameter_iterator = command_parameters.find("xoffset");
		ParametersMap::const_iterator yoffset_parameter_iterator = command_parameters.find("yoffset");
		bool element_specified(element_parameter_iterator != command_parameters.end());
		bool offset_specified((xoffset_parameter_iterator != command_parameters.end()) && (yoffset_parameter_iterator != command_parameters.end()));
		if (!element_specified && !offset_specified) {
			response->SetErrorResponse(400, "Missing parameters: element, xoffset, yoffset");
			return;
		} else {
			int status_code = SUCCESS;

			long start_x = session.last_known_mouse_x();
			long start_y = session.last_known_mouse_y();

			long end_x = start_x;
			long end_y = start_y;
			if (element_specified && !element_parameter_iterator->second.isNull()) {
				std::wstring element_id(CA2W(element_parameter_iterator->second.asCString(), CP_UTF8));
				status_code = this->GetElementCoordinates(session, element_id, offset_specified, &end_x, &end_y);
				if (status_code != SUCCESS) {
					response->SetErrorResponse(status_code, "Unable to locate element with id " + element_parameter_iterator->second.asString());
				}
			}

			if (offset_specified) {
				end_x += xoffset_parameter_iterator->second.asInt();
				end_y += yoffset_parameter_iterator->second.asInt();
			}

			BrowserHandle browser_wrapper;
			status_code = session.GetCurrentBrowser(&browser_wrapper);
			if (status_code != SUCCESS) {
				response->SetErrorResponse(status_code, "Unable to get current browser");
			}

			HWND browser_window_handle = browser_wrapper->GetWindowHandle();
			LRESULT move_result = mouseMoveTo(browser_window_handle, session.speed(), start_x, start_y, end_x, end_y);

			Session& mutable_session = const_cast<Session&>(session);
			mutable_session.set_last_known_mouse_x(end_x);
			mutable_session.set_last_known_mouse_y(end_y);

			response->SetResponse(SUCCESS, Json::Value::null);
			return;
		}
	}

private:
	int MouseMoveToCommandHandler::GetElementCoordinates(const Session& session, const std::wstring& element_id, bool get_element_origin, long *x_coordinate, long *y_coordinate) {
		ElementHandle target_element;
		int status_code = this->GetElement(session, element_id, &target_element);
		if (status_code != SUCCESS) {
			return status_code;
		}

		long element_x, element_y, element_width, element_height;
		status_code = target_element->GetLocationOnceScrolledIntoView(&element_x, &element_y, &element_width, &element_height);
		if (status_code == SUCCESS) {
			if (get_element_origin) {
				*x_coordinate = element_x;
				*y_coordinate = element_y;
			} else {
				*x_coordinate = element_x + (element_width / 2);
				*y_coordinate = element_y + (element_height / 2);
			}
		}

		return SUCCESS;
	}
};

} // namespace webdriver

#endif // WEBDRIVER_IE_MOUSEMOVETOCOMMANDHANDLER_H_
