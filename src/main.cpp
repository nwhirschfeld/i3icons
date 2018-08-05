#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <algorithm> // std::find_if
#include <i3ipc++/ipc.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>




#define MAXSTR 1000

Display *display;
unsigned long window;
unsigned char *prop;
std::map<std::string, std::string> iconmap;
i3ipc::connection conn;

void fill_iconmap() {
  // read icon table from config file
  std::ifstream infile("/usr/local/etc/i3icons/icons.config");
  std::string line;
  while (std::getline(infile, line))
  {

  std::cout << line << std::endl;
      auto end = line.find('=');
      iconmap[line.substr(0, end)] = line.substr(end+1, line.size());
  }
}

int check_status(int status) {
  if (status == BadWindow) {
    throw std::string("window id does not exists!");
  }

  if (status != Success) {
    throw std::string("XGetWindowProperty failed!");
  }

  return 0;
}

const char *get_string_property(std::string property_name) {
  Atom actual_type, filter_atom;
  int actual_format, status;
  unsigned long nitems, bytes_after;

  filter_atom = XInternAtom(display, property_name.c_str(), True);
  XSynchronize(display, True);
  status = XGetWindowProperty(display, window, filter_atom, 0, MAXSTR, False,
                              AnyPropertyType, &actual_type, &actual_format,
                              &nitems, &bytes_after, &prop);
  if (check_status(status)) return "";
  return (char const *) prop;
}

std::string get_window_class() {
  std::string sName (get_string_property("WM_CLASS"));
  return sName;
}

void rename_ws() {
  // prepare display for reading shortnames
  char *display_name = nullptr; // could be the value of $DISPLAY
  display = XOpenDisplay(display_name);
  if (display == nullptr) {
    fprintf(stderr, "%s:  unable to open display '%s'\n", "XXX",
            XDisplayName(display_name));
  }

  // get and parse the i3 tree
  std::list<std::shared_ptr<i3ipc::container_t>> tree = conn.get_tree()->nodes;
  for (auto &screen : tree) {
    if (screen->name != "__i3") {
      for (auto &element : screen->nodes) {
        if (element->name == "content") {
          for (auto &ws : element->nodes) {

            // collect the shortnames (or icons) of the windows
            std::list<std::string> shortnames;

            for (auto &w : ws->nodes) {
              // set the id of the current window to get the WM_CLASS
              window = w->xwindow_id;
              std::string sn = get_window_class();
              if (iconmap.find(sn) == iconmap.end()) {
                // no icon found
                shortnames.insert(shortnames.end(), sn);
              } else {
                shortnames.insert(shortnames.end(), iconmap[sn]);
              }
            }

            shortnames.unique();
            std::string sn_string = "";
            for (auto &s : shortnames) {
              sn_string.append(s + " ");
            }

            if (!conn.send_command("rename workspace \"" + ws->name + "\" to " +
                                   ws->name.substr(0, ws->name.find(":", 0)) +
                                   ": " + sn_string)) {
              throw std::string("Failed to exit via command");
            }
          }
        }
      }
    }
  }
}

int main() {
  fill_iconmap();

  // substribe window events
  conn.subscribe(i3ipc::ET_WINDOW);

  // handle window events
  conn.signal_window_event.connect([](const i3ipc::window_event_t &ev) {
    if (tolower((char)ev.type) != 'f') { // ignore Fullscreen and focus events
      rename_ws();
    }
  });
  while (true) {
    conn.handle_event();
  }
}
