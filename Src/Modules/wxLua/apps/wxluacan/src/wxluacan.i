
%include "canlua.h"
%include "cancom.h"
%include "cansim.h"
%include "wx/gdicmn.h"

%class %noclassinfo wxlCanObj, wxObject
    wxlCanObj( double x = 0, double y = 0 )
   	void SetPos( double x, double y )
    double GetX()
    double GetY()
    void SetPen( const wxPen& pen )
    void SetBrush( const wxBrush& brush )
    void SetPending( bool pending = true )
    void AddObject( wxlCanObj *canobj )
%endclass

%class %noclassinfo wxlCanObjRect, wxlCanObj
    wxlCanObjRect(  double x, double y, double w, double h )
%endclass

%class %noclassinfo wxlCanObjCircle, wxlCanObj
    wxlCanObjCircle( double x, double y, double r )
%endclass

%class %noclassinfo wxlCanObjScript, wxlCanObj
    wxlCanObjScript( double x, double y, const wxString& name )
%endclass

%class %noclassinfo wxlCanObjAddScript, wxlCanObj
    wxlCanObjAddScript( double x, double y,  const wxString& script )
    void SetScript( const wxString& script )
%endclass

%class %noclassinfo wxlCan, wxScrolledWindow
    wxlCan( wxWindow* parent, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize )
    void AddObject( wxlCanObj *canobj )
    bool GetYaxis()

    wxlLuaCanCmd* GetCmdh()
%endclass

%function wxlCan* GetCan()
%function wxlLuaCanCmd* GetCmdhMain()

%class %noclassinfo wxlLuaCanCmd, wxCommandProcessor
    wxlLuaCanCmd( wxlCan* canvas, int maxCommands = -1 )
    void MoveObject( int index, double x, double y )
%endclass


