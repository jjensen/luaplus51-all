///////////////////////////////////////////////////////////////////////////////
// Name:        stetree.h
// Purpose:     wxSTEditorTreeCtrl
// Author:      John Labenski
// Modified by:
// Created:     03/05/2012
// Copyright:   (c) John Labenski
// Licence:     wxWidgets licence
///////////////////////////////////////////////////////////////////////////////

/// @file stetree.h
/// @brief wxSTEditorTreeCtrl, a wxTreeCtrl that can sync with a wxSTEditorNotebook.

#ifndef _STETREE_H_
#define _STETREE_H_

#include "wx/stedit/stedefs.h"

#include <wx/treectrl.h>
#include <wx/hashmap.h>

//-----------------------------------------------------------------------------
/// @class wxSTETreeItemData
/// @brief wxTreeItemData for the wxSTEditorTreeCtrl wxTreeCtrl file list.
//-----------------------------------------------------------------------------

class wxSTETreeItemData : public wxTreeItemData
{
public:
    wxSTETreeItemData(int page_num = -1, wxWindow* win = NULL);

    virtual ~wxSTETreeItemData();

    wxTreeItemId  m_id;         ///< The tree id in the wxTreeCtrl.
    int           m_page_num;   ///< The notebook page number, or -1 for none.
    wxWindow*     m_notePage;   ///< The wxNotebook's wxWindow page
                                ///<   most likely a wxSTEditorSplitter/wxSTEditor.
    wxString      m_root;       ///< Root leaf in the wxTreeCtrl.
    wxFileName    m_fileName;   ///< Filename of the page.
    wxArrayString m_treePath;   ///< Path to the tree item, without root item.

    wxSTEditorRefData* m_steRefData; ///< If a node for a wxSTEditor, its data.
};

/*
WX_DEFINE_ARRAY(wxSTETreeItemData *, wxArraySTETreeItemData);

int wxCMPFUNC_CONV STE_TreeItemSortCompareFunction( wxSTETreeItemData** first, wxSTETreeItemData** second)
{
    int ret = wxStrcmp((*first)->m_root, (*second)->m_root);
    if (ret == 0)
        ret = wxStrcmp((*first)->m_fileName, (*second)->m_fileName);
    return ret;
}
*/

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

/// Options for the wxSTETreeCtrl::GetAllItemIds() function.
enum STE_TreeCtrlGet_Type
{
    STE_TREECTRL_GET_DATA   = 1, // get items that have data
    STE_TREECTRL_GET_NODATA = 2, // get items that don't have data
    STE_TREECTRL_GET_ALL    = 3  // get all items
};

/// Options for the wxSTETreeCtrl::FindOrInsertItem() function.
enum STE_TreeCtrlFindInsert_Type
{
    STE_TREECTRL_FIND           = 1, // only find an existing item
    STE_TREECTRL_INSERT         = 2, // just insert the item, even if one with same path exists
    STE_TREECTRL_FIND_OR_INSERT = 3  // try to find existing, else insert
};

/// Indexes into the wxImageList assigned to the wxSTETreeCtrl.
enum STE_TreeCtrlImage_Type
{
    STE_TREECTRL_IMAGE_FOLDER = 0,
    STE_TREECTRL_IMAGE_EDITOR,
    STE_TREECTRL_IMAGE_OTHER
};

//-----------------------------------------------------------------------------
/// @class wxSTEditorTreeCtrl
/// @brief A wxTreeCtrl that can sync with the wxSTEditorNotebook class.
//-----------------------------------------------------------------------------

class WXDLLIMPEXP_STEDIT wxSTEditorTreeCtrl : public wxTreeCtrl
{
public:
    wxSTEditorTreeCtrl() : wxTreeCtrl() { Init(); }
    wxSTEditorTreeCtrl(wxWindow *parent, wxWindowID id = wxID_ANY,
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize,
                       long style = wxTR_SINGLE|wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT,
                       const wxString& name = wxT("wxSTEditorTreeCtrl")) : wxTreeCtrl()
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }

    virtual ~wxSTEditorTreeCtrl();

    bool Create(wxWindow *parent, wxWindowID id = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTR_SINGLE|wxTR_HAS_BUTTONS|wxTR_HIDE_ROOT|wxTR_LINES_AT_ROOT,
                const wxString& name = wxT("wxSTEditorTreeCtrl"));

    /// Set the wxSTEditorNotebook to show the pages for.
    void SetSTENotebook(wxSTEditorNotebook* notebook);
    /// Get the wxSTEditorNotebook this syncs with.
    wxSTEditorNotebook* GetSTEditorNotebook() { return m_steNotebook; }

    /// How to display the filenames in the tree.
    /// For "c:\software\program\readme.txt", the tree will show the
    ///   values as described.
    enum FileDisplay_Type
    {
        SHOW_FILENAME_ONLY,      ///< Nodes only with the filename, i.e. 'readme.txt'.
        SHOW_FILEPATH_ONLY,      ///< Nodes only with the full path, i.e. 'c:\software\program\readme.txt'.
        SHOW_PATH_THEN_FILENAME, ///< Nodes of 'c:\software\program\' child node of -> 'readme.txt'.
        SHOW_ALL_PATHS           ///< Nodes of 'c:\' -> 'software' -> 'program' -> 'readme.txt'.
    };

    /// How to display the filename information in the tree.
    void SetDisplayType(FileDisplay_Type display_type);

    /// Update the treectrl from the set wxSTEditorNotebook.
    /// This should not need to be called by a user.
    void UpdateFromNotebook();

    /// Recursively sort all children of the item.
    void SortAllChildren(const wxTreeItemId& item);

    /// Get the "path" to the item by traversing up the tree from the item
    /// and adding each label of the treectrl to the array.
    wxArrayString GetItemPath(const wxTreeItemId& id);

    /// Find or creates paths to insert the item.
    /// Each index in array is a leaf the last array element is the item itself.
    /// If only find then if the item doesn't exist an invalid wxTreeItemId is returned.
    /// @returns The existing or new item id or invalid one if it didn't exist.
    wxTreeItemId FindOrInsertItem(const wxArrayString& treePath,
                                  STE_TreeCtrlFindInsert_Type = STE_TREECTRL_FIND_OR_INSERT);

    /// Delete the item with the given path.
    /// If delete_empty then remove parents that are empty after removing the item.
    bool DeleteItem(const wxArrayString& treePath, bool delete_empty = true);

    /// Delete the item with the given id.
    /// Travel up the parents if delete_empty deleting item + parents the number of levels up.
    /// levels = -1 means go all the way up to the root tree item.
    /// topId if valid is the highest treeId to traverse to and stop at.
    /// @returns The # of nodes deleted.
    int DeleteItem(const wxTreeItemId& id, bool delete_empty, int levels = -1,
                   const wxTreeItemId& topId = wxTreeItemId());

    /// Get all the treectrl items in an array, type determines what items to add.
    size_t GetAllChildrenItemIds(const wxTreeItemId& start_id,
                                 wxArrayTreeItemIds& arrayIds,
                                 STE_TreeCtrlGet_Type = STE_TREECTRL_GET_ALL);
    /// Internal use recursive function to get all children.
    size_t DoGetAllChildrenItemIds(const wxTreeItemId& start_id,
                                   wxArrayTreeItemIds& arrayIds,
                                   STE_TreeCtrlGet_Type = STE_TREECTRL_GET_ALL);

    /// Overridden wxTreeCtrl methods
    virtual int OnCompareItems(const wxTreeItemId& item1, const wxTreeItemId& item2);

    /// @name Event handlers
    /// @{
    void OnContextMenu( wxContextMenuEvent& event );
    void OnMenu( wxCommandEvent& event );
    bool HandleMenuEvent(wxCommandEvent &event);
    void OnTreeCtrl(wxTreeEvent &event);
    void OnNotebookPageChanged( wxNotebookEvent &event );
    void OnSTEState( wxSTEditorEvent &event );
    void OnWindowDestroy( wxWindowDestroyEvent& event );
    /// @}

protected:
    FileDisplay_Type    m_display_type;
    wxTreeItemId        m_notePageId;

    wxMenu*             m_popupMenu;
    wxSTEditorNotebook* m_steNotebook;
    wxLongToLongHashMap m_windowToSTETreeItemDataMap;
    wxLongToLongHashMap m_windowDestroyMap;

    wxSTERecursionGuardFlag m_rGuard_UpdateFromNotebook;

private:
    void Init();
    DECLARE_DYNAMIC_CLASS(wxSTEditorTreeCtrl)
    DECLARE_EVENT_TABLE()
};


#endif  // _STETREE_H_
