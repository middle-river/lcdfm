/*
  LCD File Manager

  T. Nakagawa
  2014/01/18  Version 1
  2020/01/25  Version 2
*/

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <dirent.h>
#include <locale.h>
#include <ncursesw/ncurses.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHECK(cond, msg) do {if (!(cond)) {fprintf(stderr, "ERROR(%s:%d): %s\n", __FILE__, __LINE__, (msg)); exit(1);}} while (false);

// Read a file with associated applications for extensions.
std::map<std::string, std::string> ReadExtensions(const std::string &file) {
  std::map<std::string, std::string> extensions;
  std::ifstream ifs(file.c_str());
  CHECK(ifs, "Cannot open the configuration file.");
  std::string line;
  while (std::getline(ifs, line)) {
    const auto pos = line.find('\t');
    CHECK(pos != std::string::npos, "Invalid configuration file.");
    extensions[line.substr(0, pos)] = line.substr(pos + 1);
  }
  return extensions;
}

// Shorten the given string in UTF-8 to fit in the specified width.
std::string Clip(const std::string &str, int limit) {
  int len = 0;
  size_t ptr = 0;
  while (ptr < str.size()) {
    const int width = ((str[ptr] & 0x80) != 0) ? 2 : 1;
    if (len + width > limit) break;
    len += width;
    ptr++;
    if (width == 2) while (ptr < str.size() && (str[ptr] & 0xc0) == 0x80) ptr++;
  }
  return str.substr(0, ptr);
}

class Path {
public:
  static std::string Basename(const std::string &path) {
    const auto pos = path.rfind('.');
    return (pos == std::string::npos) ? "" : path.substr(0, pos);
  }

  static std::string Extname(const std::string &path) {
    const auto pos = path.rfind('.');
    return (pos == std::string::npos) ? "" : path.substr(pos + 1);
  }

  static std::string Parentdir(const std::string &path) {
    const auto pos = path.rfind('/');
    return (pos == std::string::npos) ? "" : path.substr(0, pos);
  }

  static void GetFiles(const std::string &path, std::vector<std::pair<std::string, bool>> *files) {
    files->clear();
    DIR *dp = opendir(path.c_str());
    if (dp == NULL) return;
    while (struct dirent *dent = readdir(dp)) {
      if (dent->d_name[0] == '.' || !(dent->d_type == DT_REG || dent->d_type == DT_DIR)) continue;
      files->push_back({dent->d_name, (dent->d_type == DT_DIR)});
    }
    std::sort(files->begin(), files->end());
  }
};

class Curses {
public:
  Curses() {
  }

  void begin() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    curs_set(0);
    noecho();
    init_pair(1, COLOR_MAGENTA, COLOR_YELLOW);
    init_pair(2, COLOR_WHITE, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_WHITE, COLOR_BLUE);
    init_pair(5, COLOR_CYAN, COLOR_BLUE);
    init_pair(6, COLOR_RED, COLOR_GREEN);
    win_[0] = newwin(LINES, COLS, 0, 0);
    win_[1] = subwin(win_[0], 1, COLS, 0, 0);
    wbkgd(win_[1], COLOR_PAIR(1));
    win_[2] = subwin(win_[0], LINES - 2, COLS, 1, 0);
    wbkgd(win_[2], COLOR_PAIR(2));
    win_[3] = subwin(win_[0], 1, COLS - 1, LINES - 1, 0);
    wbkgd(win_[3], COLOR_PAIR(6));
    refresh();
  }

  void end() {
    endwin();
  }

  void DrawScreen(const std::string &dir, const std::vector<std::pair<std::string, bool>> &files, int cursor) {
    std::string buf;

    /* Title window */
    buf = Clip(dir, COLS);
    mvwaddstr(win_[1], 0, 0, buf.c_str());
    wclrtoeol(win_[1]);
    wrefresh(win_[1]);

    /* Status window */
    buf = files[cursor].second ? "" : Path::Extname(files[cursor].first);
    buf = Clip(buf, COLS - 1);
    mvwaddstr(win_[3], 0, 0, buf.c_str());
    wclrtoeol(win_[3]);
    buf = std::to_string(cursor + 1) + "/" + std::to_string(files.size());
    buf = Clip(buf, COLS - 1);
    mvwaddstr(win_[3], 0, COLS - buf.size() - 1, buf.c_str());
    wrefresh(win_[3]);

    /* Main window */
    const int offset = (cursor / (LINES - 2)) * (LINES - 2);
    for (int i = 0; i < LINES - 2; i++) {
      wmove(win_[2], i, 0);
      if (offset + i >= (int)files.size()) {
	wclrtobot(win_[2]);
	break;
      }
      buf = files[offset + i].first;
      if (files[offset + i].second) {
	if (offset + i == cursor) {
	  wattron(win_[2], COLOR_PAIR(5));
	} else {
	  wattron(win_[2], COLOR_PAIR(3));
	}
      } else {
	buf = Path::Basename(buf);
	if (offset + i == cursor) {
	  wattron(win_[2], COLOR_PAIR(4));
	} else {
	  wattron(win_[2], COLOR_PAIR(2));
	}
      }
      buf = Clip(buf, COLS);
      waddstr(win_[2], buf.c_str());
      wclrtoeol(win_[2]);
    }
    wrefresh(win_[2]);

    refresh();
  }

private:
  WINDOW *win_[4];
} Curses;

int main(int argc, char **argv) {
  CHECK(argc == 3, "usage: lcdfm <root directory> <configuration file>");
  std::string root_dir(argv[1]);
  const auto extensions = ReadExtensions(argv[2]);

  std::string path(root_dir);
  int cursor = 0;
  std::vector<int> cursor_hist;  // The cursor positions in the past.
  std::vector<std::pair<std::string, bool>> files;
  Path::GetFiles(path, &files);
  CHECK(!files.empty(), "No files exist in the root directory.");
  Curses.begin();
  Curses.DrawScreen(path.substr(root_dir.size()), files, cursor);

  for (; ; ) {
    const char c = getch();
    if (c == 'q') {
      break;
    } else if (c == 'h' && !cursor_hist.empty()) {  /* Move to the parent directory. */
      path = Path::Parentdir(path);
      CHECK(!path.empty(), "Invalid path name.");
      Path::GetFiles(path, &files);
      CHECK(!files.empty(), "No files exist.");
      cursor = cursor_hist.back();
      cursor_hist.pop_back();
    } else if (c == 'l') {  // Move to a child directory or execute a file.
      if (files[cursor].second) {  // Directory.
	const std::string tmp_path = path + "/" + files[cursor].first;
	std::vector<std::pair<std::string, bool>> tmp_files;
	Path::GetFiles(tmp_path, &tmp_files);
	if (tmp_files.empty()) {
	  continue;
	} else {
	  path = tmp_path;
	  files = tmp_files;
	  cursor_hist.push_back(cursor);
	  cursor = 0;
	}
      } else {  // File.
	const auto it = extensions.find(Path::Extname(files[cursor].first));
	if (it == extensions.end()) continue;
	Curses.end();
	const int pid = fork();
	if (pid == 0) {	/* Child process. */
	  path += "/" + files[cursor].first;
	  execl(it->second.c_str(), it->second.c_str(), path.c_str(), NULL);
	  return 1;
	} else if (pid > 0) {	/* Parent process. */
	  wait(NULL);
	}
	Curses.begin();
      }
    } else if (c == 'j' && cursor + 1 < (int)files.size()) {  // Cursor down.
      cursor++;
    } else if (c == 'k' && cursor - 1 >= 0) {  // Cursor up.
      cursor--;
    } else {
      continue;
    }

    Curses.DrawScreen(path.substr(root_dir.size()), files, cursor);
  }

  Curses.end();

  return 0;
}
