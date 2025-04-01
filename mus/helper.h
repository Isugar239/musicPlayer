#include <string>
#include <vector>
#include <iostream>

bool openFile();
bool OpenFileDialog(std::vector<std::wstring> &paths, bool selectFolder = false, bool multiSelect = false);
