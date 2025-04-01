#include <iostream>
#include <windows.h>
#include <shobjidl.h>
#include <string>
#include <vector>

/**
 * @brief Open a dialog to select item(s) or folder(s).
 * @param paths Specifies the reference to the string vector that will receive the file or folder path(s). [IN]
 * @param selectFolder Specifies whether to select folder(s) rather than file(s). (optional)
 * @param multiSelect Specifies whether to allow the user to select multiple items. (optional)
 * @note If no item(s) were selected, the function still returns true, and the given vector is unmodified.
 * @note `<windows.h>`, `<string>`, `<vector>`, `<shobjidl.h>`
 * @return Returns true if all the operations are successfully performed, false otherwise.
 */
bool OpenFileDialog(std::vector<std::wstring> &paths, bool selectFolder = false, bool multiSelect = false)
{
    IFileOpenDialog *p_file_open = nullptr;
    bool are_all_operation_success = false;
    while (!are_all_operation_success)
    {
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                      IID_IFileOpenDialog, reinterpret_cast<void **>(&p_file_open));
        if (FAILED(hr))
            break;

        if (selectFolder || multiSelect)
        {
            FILEOPENDIALOGOPTIONS options = 0;
            hr = p_file_open->GetOptions(&options);
            if (FAILED(hr))
                break;

            if (selectFolder)
                options |= FOS_PICKFOLDERS;
            if (multiSelect)
                options |= FOS_ALLOWMULTISELECT;

            hr = p_file_open->SetOptions(options);
            if (FAILED(hr))
                break;
        }

        hr = p_file_open->Show(NULL);
        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) // No items were selected.
        {
            are_all_operation_success = true;
            break;
        }
        else if (FAILED(hr))
            break;

        IShellItemArray *p_items;
        hr = p_file_open->GetResults(&p_items);
        if (FAILED(hr))
            break;
        DWORD total_items = 0;
        hr = p_items->GetCount(&total_items);
        if (FAILED(hr))
            break;

        for (int i = 0; i < static_cast<int>(total_items); ++i)
        {
            IShellItem *p_item;
            p_items->GetItemAt(i, &p_item);
            if (SUCCEEDED(hr))
            {
                PWSTR path;
                hr = p_item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                if (SUCCEEDED(hr))
                {
                    paths.push_back(path);
                    CoTaskMemFree(path);
                }
                p_item->Release();
            }
        }

        p_items->Release();
        are_all_operation_success = true;
    }

    if (p_file_open)
        p_file_open->Release();
    return are_all_operation_success;
}

bool openFile(){
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    std::vector<std::wstring> paths;
//     CoUninitialize();
   return OpenFileDialog(paths);
}
