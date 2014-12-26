///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2013 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#define LUA_CORE
#include "LuaPlus.h"

#include <ctype.h>

using namespace LuaPlus;

#if LUA_VERSION_NUM >= 502
static int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}

#endif

/* macro to `unsign' a character */
#define uchar(c)        ((unsigned char)(c))

extern "C" int str_format_helper (luaL_Buffer* b, lua_State *L, int arg);

namespace LuaPlus {

static void luaI_addquotednonwidebinary (LuaStateOutFile& file, const char* s, size_t l) {
	file.Print("%c", '"');
	while (l--) {
		switch (*s) {
			case '"':  case '\\':
				file.Print("\\%c", *s);
				break;
			case '\a':		file.Print("\\a");		break;
			case '\b':		file.Print("\\b");		break;
			case '\f':		file.Print("\\f");		break;
			case '\n':		file.Print("\\n");		break;
			case '\r':		file.Print("\\r");		break;
			case '\t':		file.Print("\\t");		break;
			case '\v':		file.Print("\\v");		break;
			default:
				if (isprint((unsigned char)*s))
					file.Print("%c", *s);
				else
					file.Print("\\x%02x", (unsigned int)(unsigned char)*s);
		}
		s++;
	}
	file.Print("%c", '"');
}


#if LUA_VERSION_NUM <= 501

#define L_ESC		'%'

/* maximum size of each formatted item (> len(format('%99.99f', -1e308))) */
#define MAX_ITEM	512
/* valid flags in a format specification */
#define FLAGS	"-+ #0"
/*
** maximum size of each format specification (such as '%-099.99d')
** (+10 accounts for %99.99x plus margin of error)
*/
#define MAX_FORMAT	(sizeof(FLAGS) + sizeof(LUA_INTFRMLEN) + 10)

static const char *scanformat (lua_State *L, const char *strfrmt, char *form) {
  const char *p = strfrmt;
  while (*p != '\0' && strchr(FLAGS, *p) != NULL) p++;  /* skip flags */
  if ((size_t)(p - strfrmt) >= sizeof(FLAGS))
    luaL_error(L, "invalid format (repeated flags)");
  if (isdigit(uchar(*p))) p++;  /* skip width */
  if (isdigit(uchar(*p))) p++;  /* (2 digits at most) */
  if (*p == '.') {
    p++;
    if (isdigit(uchar(*p))) p++;  /* skip precision */
    if (isdigit(uchar(*p))) p++;  /* (2 digits at most) */
  }
  if (isdigit(uchar(*p)))
    luaL_error(L, "invalid format (width or precision too long)");
  *(form++) = '%';
  strncpy(form, strfrmt, p - strfrmt + 1);
  form += p - strfrmt + 1;
  *form = '\0';
  return p;
}


static void addquoted (lua_State *L, luaL_Buffer *b, int arg) {
  size_t l;
  const char *s = luaL_checklstring(L, arg, &l);
  luaL_addchar(b, '"');
  while (l--) {
    switch (*s) {
      case '"': case '\\': case '\n': {
        luaL_addchar(b, '\\');
        luaL_addchar(b, *s);
        break;
      }
      case '\r': {
        luaL_addlstring(b, "\\r", 2);
        break;
      }
      case '\0': {
        luaL_addlstring(b, "\\000", 4);
        break;
      }
      default: {
        luaL_addchar(b, *s);
        break;
      }
    }
    s++;
  }
  luaL_addchar(b, '"');
}


void addquotedbinary (lua_State *L, luaL_Buffer *b, int arg) {
  size_t l;
  {
    const char *s = luaL_checklstring(L, arg, &l);
    luaL_addchar(b, '"');
    while (l--) {
      switch (*s) {
        case '"':  case '\\':
          luaL_addchar(b, '\\');
          luaL_addchar(b, *s);
          break;
        case '\a':  luaL_addchar(b, '\\');  luaL_addchar(b, 'a');  break;
        case '\b':  luaL_addchar(b, '\\');  luaL_addchar(b, 'b');  break;
        case '\f':  luaL_addchar(b, '\\');  luaL_addchar(b, 'f');  break;
        case '\n':  luaL_addchar(b, '\\');  luaL_addchar(b, 'n');  break;
        case '\r':  luaL_addchar(b, '\\');  luaL_addchar(b, 'r');  break;
        case '\t':  luaL_addchar(b, '\\');  luaL_addchar(b, 't');  break;
        case '\v':  luaL_addchar(b, '\\');  luaL_addchar(b, 'v');  break;
        default:
          if (isprint((unsigned char)*s)) {
            luaL_addchar(b, *s);
          } else {
            char str[10];
            sprintf(str, "\\x%02x", (unsigned int)(unsigned char)*s);
            luaL_addstring(b, str);
          }
      }
      s++;
    }
    luaL_addchar(b, '"');
  }
}


static void addintlen (char *form) {
  size_t l = strlen(form);
  char spec = form[l - 1];
  strcpy(form + l - 1, LUA_INTFRMLEN);
  form[l + sizeof(LUA_INTFRMLEN) - 2] = spec;
  form[l + sizeof(LUA_INTFRMLEN) - 1] = '\0';
}


int str_format_helper (luaL_Buffer *b, lua_State *L, int arg) {
  int top = lua_gettop(L);
  size_t sfl;
  const char *strfrmt = luaL_checklstring(L, arg, &sfl);
  const char *strfrmt_end = strfrmt+sfl;
  luaL_buffinit(L, b);
  while (strfrmt < strfrmt_end) {
    if (*strfrmt != L_ESC)
      luaL_addchar(b, *strfrmt++);
    else if (*++strfrmt == L_ESC)
      luaL_addchar(b, *strfrmt++);  /* %% */
    else { /* format item */
      char form[MAX_FORMAT];  /* to store the format (`%...') */
      char buff[MAX_ITEM];  /* to store the formatted item */
      if (++arg > top)
        luaL_argerror(L, arg, "no value");
      strfrmt = scanformat(L, strfrmt, form);
      switch (*strfrmt++) {
        case 'c': {
          sprintf(buff, form, (int)luaL_checknumber(L, arg));
          break;
        }
        case 'd':  case 'i': {
          addintlen(form);
          sprintf(buff, form, (LUA_INTFRM_T)luaL_checknumber(L, arg));
          break;
        }
        case 'o':  case 'u':  case 'x':  case 'X': {
          addintlen(form);
          sprintf(buff, form, (unsigned LUA_INTFRM_T)luaL_checknumber(L, arg));
          break;
        }
        case 'e':  case 'E': case 'f':
        case 'g': case 'G': {
          sprintf(buff, form, (double)luaL_checknumber(L, arg));
          break;
        }
        case 'q': {
          addquoted(L, b, arg);
          continue;  /* skip the 'addsize' at the end */
        }
        case 's': {
          size_t l;
          const char *s = luaL_checklstring(L, arg, &l);
          if (!strchr(form, '.') && l >= 100) {
            /* no precision and string is too long to be formatted;
               keep original string */
            lua_pushvalue(L, arg);
            luaL_addvalue(b);
            continue;  /* skip the `addsize' at the end */
          }
          else {
            sprintf(buff, form, s);
            break;
          }
        }
        default: {  /* also treat cases `pnLlh' */
          return luaL_error(L, "invalid option " LUA_QL("%%%c") " to "
                               LUA_QL("format"), *(strfrmt - 1));
        }
        case 'B': {
          if (!lua_isboolean(L, arg)  &&  !lua_isnil(L, arg))
            luaL_typerror(L, arg, lua_typename(L, LUA_TBOOLEAN));
          strcpy(buff, lua_toboolean(L, arg) ? "true" : "false");
          break;
        }
        case 'Q': {
          addquotedbinary(L, b, arg);
          continue;  /* skip the `addsize' at the end */
        }
      }
      luaL_addlstring(b, buff, strlen(buff));
    }
  }
  return 1;
}

#define bufflen(B)	((B)->p - (B)->buffer)

#else

/*
** {======================================================
** STRING FORMAT
** =======================================================
*/

/*
** LUA_INTFRMLEN is the length modifier for integer conversions in
** 'string.format'; LUA_INTFRM_T is the integer type corresponding to
** the previous length
*/
#if !defined(LUA_INTFRMLEN)	/* { */
#if defined(LUA_USE_LONGLONG)

#define LUA_INTFRMLEN		"ll"
#define LUA_INTFRM_T		long long

#else

#define LUA_INTFRMLEN		"l"
#define LUA_INTFRM_T		long

#endif
#endif				/* } */


/*
** LUA_FLTFRMLEN is the length modifier for float conversions in
** 'string.format'; LUA_FLTFRM_T is the float type corresponding to
** the previous length
*/
#if !defined(LUA_FLTFRMLEN)

#define LUA_FLTFRMLEN		""
#define LUA_FLTFRM_T		double

#endif


/* maximum size of each formatted item (> len(format('%99.99f', -1e308))) */
#define MAX_ITEM	512
/* valid flags in a format specification */
#define FLAGS	"-+ #0"
/*
** maximum size of each format specification (such as '%-099.99d')
** (+10 accounts for %99.99x plus margin of error)
*/
#define MAX_FORMAT	(sizeof(FLAGS) + sizeof(LUA_INTFRMLEN) + 10)


static void addquoted (lua_State *L, luaL_Buffer *b, int arg) {
  size_t l;
  const char *s = luaL_checklstring(L, arg, &l);
  luaL_addchar(b, '"');
  while (l--) {
    if (*s == '"' || *s == '\\' || *s == '\n') {
      luaL_addchar(b, '\\');
      luaL_addchar(b, *s);
    }
    else if (*s == '\0' || iscntrl(uchar(*s))) {
      char buff[10];
      if (!isdigit(uchar(*(s+1))))
        sprintf(buff, "\\%d", (int)uchar(*s));
      else
        sprintf(buff, "\\%03d", (int)uchar(*s));
      luaL_addstring(b, buff);
    }
    else
      luaL_addchar(b, *s);
    s++;
  }
  luaL_addchar(b, '"');
}

static const char *scanformat (lua_State *L, const char *strfrmt, char *form) {
  const char *p = strfrmt;
  while (*p != '\0' && strchr(FLAGS, *p) != NULL) p++;  /* skip flags */
  if ((size_t)(p - strfrmt) >= sizeof(FLAGS)/sizeof(char))
    luaL_error(L, "invalid format (repeated flags)");
  if (isdigit(uchar(*p))) p++;  /* skip width */
  if (isdigit(uchar(*p))) p++;  /* (2 digits at most) */
  if (*p == '.') {
    p++;
    if (isdigit(uchar(*p))) p++;  /* skip precision */
    if (isdigit(uchar(*p))) p++;  /* (2 digits at most) */
  }
  if (isdigit(uchar(*p)))
    luaL_error(L, "invalid format (width or precision too long)");
  *(form++) = '%';
  memcpy(form, strfrmt, (p - strfrmt + 1) * sizeof(char));
  form += p - strfrmt + 1;
  *form = '\0';
  return p;
}


/*
** add length modifier into formats
*/
static void addlenmod (char *form, const char *lenmod) {
  size_t l = strlen(form);
  size_t lm = strlen(lenmod);
  char spec = form[l - 1];
  strcpy(form + l - 1, lenmod);
  form[l + lm - 1] = spec;
  form[l + lm] = '\0';
}


#define L_ESC		'%'

static int str_format_helper (luaL_Buffer* b, lua_State *L, int arg) {
  int top = lua_gettop(L);
  size_t sfl;
  const char *strfrmt = luaL_checklstring(L, arg, &sfl);
  const char *strfrmt_end = strfrmt+sfl;
  luaL_buffinit(L, b);
  while (strfrmt < strfrmt_end) {
    if (*strfrmt != L_ESC)
      luaL_addchar(b, *strfrmt++);
    else if (*++strfrmt == L_ESC)
      luaL_addchar(b, *strfrmt++);  /* %% */
    else { /* format item */
      char form[MAX_FORMAT];  /* to store the format (`%...') */
      char *buff = luaL_prepbuffsize(b, MAX_ITEM);  /* to put formatted item */
      int nb = 0;  /* number of bytes in added item */
      if (++arg > top)
        luaL_argerror(L, arg, "no value");
      strfrmt = scanformat(L, strfrmt, form);
      switch (*strfrmt++) {
        case 'c': {
          nb = sprintf(buff, form, luaL_checkinteger(L, arg));
          break;
        }
        case 'd': case 'i': {
          lua_Number n = luaL_checknumber(L, arg);
          LUA_INTFRM_T ni = (LUA_INTFRM_T)n;
          lua_Number diff = n - (lua_Number)ni;
          luaL_argcheck(L, -1 < diff && diff < 1, arg,
                        "not a number in proper range");
          addlenmod(form, LUA_INTFRMLEN);
          nb = sprintf(buff, form, ni);
          break;
        }
        case 'o': case 'u': case 'x': case 'X': {
          lua_Number n = luaL_checknumber(L, arg);
          unsigned LUA_INTFRM_T ni = (unsigned LUA_INTFRM_T)n;
          lua_Number diff = n - (lua_Number)ni;
          luaL_argcheck(L, -1 < diff && diff < 1, arg,
                        "not a non-negative number in proper range");
          addlenmod(form, LUA_INTFRMLEN);
          nb = sprintf(buff, form, ni);
          break;
        }
        case 'e': case 'E': case 'f':
#if defined(LUA_USE_AFORMAT)
        case 'a': case 'A':
#endif
        case 'g': case 'G': {
          addlenmod(form, LUA_FLTFRMLEN);
          nb = sprintf(buff, form, (LUA_FLTFRM_T)luaL_checknumber(L, arg));
          break;
        }
        case 'q': {
          addquoted(L, b, arg);
          break;
        }
        case 's': {
          size_t l;
          const char *s = luaL_tolstring(L, arg, &l);
          if (!strchr(form, '.') && l >= 100) {
            /* no precision and string is too long to be formatted;
               keep original string */
            luaL_addvalue(b);
            break;
          }
          else {
            nb = sprintf(buff, form, s);
            lua_pop(L, 1);  /* remove result from 'luaL_tolstring' */
            break;
          }
        }
        default: {  /* also treat cases `pnLlh' */
          return luaL_error(L, "invalid option " LUA_QL("%%%c") " to "
                               LUA_QL("format"), *(strfrmt - 1));
        }
      }
      luaL_addsize(b, nb);
    }
  }
  luaL_pushresult(b);
  return 1;
}


#define bufflen(B)	((B)->n)


#endif

static int LS_LuaFilePrint(LuaState* state) {
	LuaStateOutFile* file = (LuaStateOutFile*)state->Stack(1).GetUserdata();

	luaL_Buffer b;
	::str_format_helper(&b, *state, 2);

	size_t l = bufflen(&b);
	if (l != 0) {
		{
			luaL_addchar(&b, 0);
#if LUA_VERSION_NUM <= 501
			file->Print(b.buffer);
#else
			file->Print(b.b);
#endif
		}
	}

	return 0;
}


static int LS_LuaFileIndent(LuaState* state) {
	LuaStateOutFile* file = (LuaStateOutFile*)state->Stack(1).GetUserdata();
	int indentLevel = (int)state->Stack(2).GetInteger();
	file->Indent((unsigned int)indentLevel);

	return 0;
}

#if LUAPLUS_DUMPOBJECT

bool LuaState::CallFormatting(LuaObject& tableObj, LuaStateOutFile& file, int indentLevel,
		bool writeAll, bool alphabetical, bool writeTablePointers, unsigned int maxIndentLevel) {
	LuaObject metatableObj = tableObj.GetMetatable();
	if (metatableObj.IsNil())
		return false;

	LuaObject formattedWriteObj = metatableObj["FormattedWrite"];
	if (!formattedWriteObj.IsFunction())
		return false;

	LuaState* state = tableObj.GetState();

	{
		LuaObject funcObj = state->GetGlobals()["LuaFilePrint"];
		if (funcObj.IsNil()) {
			state->GetGlobals().Register("LuaFilePrint", LS_LuaFilePrint);
		}

		funcObj = state->GetGlobals()["LuaFileIndent"];
		if (funcObj.IsNil()) {
			state->GetGlobals().Register("LuaFileIndent", LS_LuaFileIndent);
		}
	}

	LuaCall call = formattedWriteObj;
	call << &file << tableObj << alphabetical << indentLevel << maxIndentLevel << writeAll << writeTablePointers << LuaRun();

	return true;
}


struct KeyValue {
	LuaObject key;
	LuaObject value;

	inline bool operator<(const KeyValue& right) const {
		if (key.Type() == right.key.Type()) {
			if (key.IsBoolean()  &&  right.key.IsBoolean())
				return !key.GetBoolean();
			return key < right.key;
		}
		if (key.IsNumber())
			return true;
		if (key.IsString()  &&  !right.key.IsNumber())
			return true;
		return false;
	}
};


static void WriteKey(LuaStateOutFile& file, LuaObject& key) {
	if (key.IsNumber()) {
		char keyName[255];
		sprintf(keyName, "[%.16g]", key.GetNumber());
		file.Print("%s", keyName);
	} else if (key.IsString()) {
		size_t keyLen = key.StrLen();
		const char* ptr = key.GetString();
		const char* endPtr = ptr + keyLen;
		bool isAlphaNumeric = true;
		if (isdigit((unsigned char)*ptr))
			isAlphaNumeric = false;
		while (ptr < endPtr) {
			if (!isalnum((unsigned char)*ptr)  &&  *ptr != '_') {
				isAlphaNumeric = false;
				break;
			}
			ptr++;
		}

		if (isAlphaNumeric)
			file.Print("%s", key.GetString());
		else {
			file.Print("[");
			luaI_addquotednonwidebinary(file, key.GetString(), key.StrLen());
			file.Print("]");
		}
	} else if (key.IsBoolean()) {
		file.Print(key.GetBoolean() ? "[true]" : "[false]");
	}
}


static bool KeyValueCompare(const KeyValue& left, const KeyValue &right) {
	return left < right;
}


template<typename E>
class SimpleList
{
public:
	SimpleList()
		: m_pHead(NULL)
		, m_pTail(NULL)
	{
	}

	~SimpleList() throw()
	{
		while (m_pHead)
		{
			CNode* pKill = m_pHead;
			m_pHead = m_pHead->m_pNext;
			delete pKill;
		}

		m_pHead = NULL;
		m_pTail = NULL;
	}

	void AddTail(E& element)
	{
		CNode* pNewNode = new CNode(element);
		pNewNode->m_pPrev = m_pTail;
		pNewNode->m_pNext = NULL;

		if (m_pTail)
			m_pTail->m_pNext = pNewNode;
		else
			m_pHead = pNewNode;

		m_pTail = pNewNode;
	}

	void* GetHeadPosition() const throw()
	{
		return m_pHead;
	}

	E& GetNext( void*& pos ) throw()
	{
		CNode* pNode = (CNode*)pos;
		pos = (void*)pNode->m_pNext;
		return pNode->m_element;
	}

	// Algorithm taken from http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html by Simon Tatham.
	template <typename CompareT>
	void Sort(CompareT Compare)
	{
		CNode *q, *e;

		if (m_pHead == NULL)
			return;

		int insize = 1;

		while (1)
		{
			CNode* p = m_pHead;
			m_pHead = NULL;
			m_pTail = NULL;

			int nmerges = 0;  /* count number of merges we do in this pass */

			while (p) {
				nmerges++;  /* there exists a merge to be done */
				/* step `insize' places along from p */
				q = p;
				int psize = 0;
				for (int i = 0; i < insize; i++)
				{
					psize++;
					q = q->m_pNext;
					if (!q)
						break;
				}

				/* if q hasn't fallen off end, we have two lists to merge */
				int qsize = insize;

				/* now we have two lists; merge them */
				while (psize > 0 || (qsize > 0 && q))
				{
					/* decide whether next element of merge comes from p or q */
					if (psize == 0)
					{
						/* p is empty; e must come from q. */
						e = q;
						q = q->m_pNext;
						qsize--;
					}
					else if (qsize == 0 || !q)
					{
						/* q is empty; e must come from p. */
						e = p;
						p = p->m_pNext;
						psize--;
					}
					else if (Compare(p->m_element, q->m_element))
					{	// p < q
						/* First element of p is lower (or same);
						* e must come from p. */
						e = p;
						p = p->m_pNext;
						psize--;
					}
					else
					{
						/* First element of q is lower; e must come from q. */
						e = q;
						q = q->m_pNext;
						qsize--;
					}

					/* add the next element to the merged list */
					if (m_pTail)
						m_pTail->m_pNext = e;
					else
						m_pHead = e;
					e->m_pPrev = m_pTail;
					m_pTail = e;
				}

				/* now p has stepped `insize' places along, and q has too */
				p = q;
			}

			m_pTail->m_pNext = NULL;

			/* If we have done only one merge, we're finished. */
			if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
			{
				return;
			}

			/* Otherwise repeat, merging lists twice the size */
			insize *= 2;
		}
	}

private:
	class CNode
	{
	public:
		CNode( E& element ) : m_element( element )  { }

		CNode* m_pNext;
		CNode* m_pPrev;
		E m_element;

	private:
		CNode( const CNode& ) throw();
	};

	CNode* m_pHead;
	CNode* m_pTail;

	SimpleList( const SimpleList& ) throw();
	SimpleList& operator=( const SimpleList& ) throw();
};


/**
	Writes a Lua object to a text file.
**/
bool LuaState::DumpObject(LuaStateOutFile& file, LuaObject& key, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	bool alreadyDumpedKey = (flags & 0xF0000000) != 0;
	flags &= ~0xF0000000;

	// If the value is nil, don't write it.
	if (value.IsNil())
		return false;

	// Indent the line the number of spaces for the current indentation level.
	const unsigned int INDENT_SIZE = 1;
	const unsigned int indentSpaces = (indentLevel == -1 ? 0 : indentLevel) * INDENT_SIZE;

	// If the variable is user data or a function...
	if (!alreadyDumpedKey  &&  (value.IsUserdata()  ||  value.IsFunction()  ||  value.IsCFunction()))
	{
		// ... only write it if they requested it be written.  But only do
		// it as a comment.
		if (flags & DUMP_WRITEALL)
		{
			if ((unsigned int)indentLevel < maxIndentLevel)
				file.Indent(indentSpaces);
			else
				file.Print(" ");

			if (value.IsUserdata())
			{
				file.Print("-- ");
				if (!key.IsNil())
				{
					WriteKey(file, key);
					file.Print(" = ");
				}
				file.Print("'userdata: %p'", value.GetUserdata());
			}
			else if (value.IsCFunction())
			{
				file.Print("-- ");
				if (!key.IsNil())
				{
					WriteKey(file, key);
					file.Print(" = ");
				}
				file.Print("'cfunction: %p'", value.GetCFunction());
			}
			else if (value.IsFunction())
			{
				lua_Debug ar;
				value.Push(this);
				lua_getinfo(*this, ">S", &ar);
//				printf("%d\n", ar.linedefined);
				file.Print("-- ");
				if (!key.IsNil())
				{
					WriteKey(file, key);
					file.Print(" = ");
				}
				file.Print("'function: %s %d'", ar.source, ar.linedefined);
			}

			return true;
		}

		return false;
	}

	if (!alreadyDumpedKey)
	{
		if ((unsigned int)indentLevel < maxIndentLevel)
			file.Indent(indentSpaces);
		else
			file.Print(" ");

		// If the object has a name, write it out.
		if (!key.IsNil())
		{
			WriteKey(file, key);

			file.Print(" = ");
		}
	}

	// If the object's value is a number, write it as a number.
	if (value.IsBoolean())
		file.Print("%s", value.GetBoolean() ? "true" : "false");

	else if (value.IsNumber())
		file.Print(LUA_NUMBER_FMT, value.GetNumber());

	// Or if the object's value is a string, write it as a quoted string.
	else if (value.IsString())
	{
		luaI_addquotednonwidebinary(file, value.GetString(), value.StrLen());
	}

	// Otherwise, see if the object's value is a table.
	else if (value.IsTable())
	{
		bool calledFormatting =
				CallFormatting(value, file, indentLevel, (flags & DUMP_WRITEALL) != 0,
					(flags & DUMP_ALPHABETICAL) != 0, (flags & DUMP_WRITETABLEPOINTERS) != 0, maxIndentLevel);
		if (!calledFormatting)
		{
			// Write the table header.
			if (indentLevel != -1)
			{
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\n");
					file.Indent(indentSpaces);
				}
				if (flags & DUMP_WRITETABLEPOINTERS)
					file.Print("{ --%8x\n", value.GetLuaPointer());
				else
					file.Print("{");
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\n");
				}
			}

			// Rename, just for ease of reading.
			LuaObject& table = value;

			// upperIndex is the upper index value of a sequential numerical array
			// items.
			int upperIndex = 1;
			bool wroteSemi = false;
			bool hasSequential = false;

			// Block to search for array items.
			{
				// Grab index 1 and index 2 of the table.
				LuaObject value1 = table.RawGetByIndex(1);
				LuaObject value2 = table.RawGetByIndex(2);

				// If they both exist, then there is a sequential list.
				if (!value1.IsNil())
				{
					// Cycle through the list.
					bool headSequential = true;
					for (; ; ++upperIndex)
					{
						// Try retrieving the table entry at upperIndex.
						LuaObject value = table.RawGetByIndex(upperIndex);

						// If it doesn't exist, then exit the loop.
						if (value.IsNil())
							break;

						// Only add the comma and return if not on the head item.
						if (!headSequential  &&  indentLevel != -1)
						{
							file.Print(",");
							if ((unsigned int)indentLevel + 1 < maxIndentLevel)
							{
								file.Print("\n");
							}
						}

						// Write the object as an unnamed entry.
						LuaObject nilObj(this);
						DumpObject(file, nilObj, value, flags, indentLevel + 1, maxIndentLevel);

						// We've definitely passed the head item now.
						headSequential = false;
					}
				}
			}

			// Did we find any sequential table values?
			if (upperIndex > 1)
			{
				hasSequential = true;
			}

			if (flags & DUMP_ALPHABETICAL)
			{
				SimpleList<KeyValue> keys;

				// Cycle through the table.
				for (LuaTableIterator it(table); it; ++it)
				{
					// Retrieve the table entry's key and value.
					LuaObject& key = it.GetKey();

					// Is the key a number?
					if (key.IsNumber())
					{
						// Yes, were there sequential array items in this table?
						if (hasSequential)
						{
							// Is the array item's key an integer?
							lua_Number realNum = key.GetNumber();
							int intNum = (int)realNum;
							if (realNum == (lua_Number)intNum)
							{
								// Yes.  Is it between 1 and upperIndex?
								if (intNum >= 1  &&  intNum < upperIndex)
								{
									// We already wrote it as part of the sequential
									// list.
									continue;
								}
							}
						}
					}

					KeyValue info;
					info.key = key;
					info.value = it.GetValue();
					keys.AddTail(info);
				}

				keys.Sort(KeyValueCompare);

				if (keys.GetHeadPosition() != NULL)
				{
					// If we wrote a sequential list, the value we're about to write
					// is not nil, and we haven't written the semicolon to separate
					// the sequential table entries from the keyed table entries...
					if (hasSequential  &&  indentLevel != -1)
					{
						// Then add a comma (for good measure).
						file.Print(", ");
						if ((unsigned int)indentLevel + 1 < maxIndentLevel)
						{
							file.Print("\n");
						}
						wroteSemi = true;
					}
				}

				for (void* keysIt = keys.GetHeadPosition(); keysIt; )
				{
					KeyValue& info = keys.GetNext(keysIt);

					// Write the table entry.
					bool ret = DumpObject(file, info.key, info.value, flags,
							indentLevel + 1, maxIndentLevel);

					// Add a comma after the table entry.
					if (indentLevel != -1  &&  ret)
					{
						file.Print(",");
						if ((unsigned int)indentLevel + 1 < maxIndentLevel)
						{
							file.Print("\n");
						}
					}
				}
			}
			else
			{
				// Cycle through the table.
				for (LuaTableIterator it(table); it; ++it)
				{
					// Retrieve the table entry's key and value.
					LuaObject& key = it.GetKey();

					// Is the key a number?
					if (key.IsNumber())
					{
						// Yes, were there sequential array items in this table?
						if (hasSequential)
						{
							// Is the array item's key an integer?
							lua_Number realNum = key.GetNumber();
							int intNum = (int)realNum;
							if (realNum == (lua_Number)intNum)
							{
								// Yes.  Is it between 1 and upperIndex?
								if (intNum >= 1  &&  intNum < upperIndex)
								{
									// We already wrote it as part of the sequential
									// list.
									continue;
								}
							}
						}
					}

					// If we wrote a sequential list, the value we're about to write
					// is not nil, and we haven't written the semicolon to separate
					// the sequential table entries from the keyed table entries...
					if (hasSequential  &&  !value.IsNil()  &&  !wroteSemi)
					{
						// Then add a comma (for good measure).
						if (indentLevel != -1)
						{
							file.Print(", ");
							if ((unsigned int)indentLevel + 1 < maxIndentLevel)
							{
								file.Print("\n");
							}
						}
						wroteSemi = true;
					}

					// Write the table entry.
					bool ret = DumpObject(file, key, it.GetValue(), flags,
						indentLevel + 1, maxIndentLevel);

					// Add a comma after the table entry.
					if (ret  &&  indentLevel != -1)
					{
						file.Print(",");
						if ((unsigned int)indentLevel + 1 < maxIndentLevel)
						{
							file.Print("\n");
						}
					}
				}
			}

			// If we wrote a sequential list and haven't written a semicolon, then
			// there were no keyed table entries.  Just write the final comma.
			if (hasSequential  &&  !wroteSemi  &&  indentLevel != -1)
			{
				file.Print(",");
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\n");
				}
			}

			// Indent, with the intent of closing up the table.
			file.Indent(indentSpaces);

			// If the indentation level is 0, then we're at the root position.
			if (indentLevel == 0)
			{
				// Add a couple extra returns for readability's sake.
				file.Print("}");
				if ((unsigned int)indentLevel + 1 < maxIndentLevel)
				{
					file.Print("\n\n");
				}
			}
			else if (indentLevel > 0)
			{
				// Close the table.  The comma is written when WriteObject()
				// returns from the recursive call.
				file.Print("}");
			}
		}
	}

	// If the indentation level is at the root, then add a return to separate
	// the lines.
	if (indentLevel == 0)
	{
		if ((unsigned int)indentLevel < maxIndentLevel)
		{
			file.Print("\n");
		}
	}

	return true;
}


/**
	Writes a Lua object to a text file.
**/
bool LuaState::DumpObject(LuaStateOutFile& file, const char* name, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	// Yes, this is hack-ish.

	// If the value is nil, don't write it.
	if (value.IsNil())
		return false;

	const unsigned int INDENT_SIZE = 1;
	const unsigned int indentSpaces = (indentLevel == -1 ? 0 : indentLevel) * INDENT_SIZE;

	// If the variable is user data or a function...
	if (value.IsUserdata()  ||  value.IsFunction()  ||  value.IsCFunction())
	{
		// ... only write it if they requested it be written.  But only do
		// it as a comment.
		if ((flags & DUMP_WRITEALL)  &&  name)
		{
			if (value.IsUserdata())
			{
				file.Print("-- %s", name);
				file.Print(" = '!!!USERDATA!!!'\n");
			}
			else if (value.IsFunction())
			{
				lua_Debug ar;
				value.Push(this);
				lua_getinfo(*this, ">S", &ar);
//				printf("%d\n", ar.linedefined);
				file.Print("-- %s", name);
				file.Print(" = '!!!FUNCTION!!! %s %d'\n", ar.source, ar.linedefined);
			}
			else
			{
				file.Print("-- %s", name);
				file.Print(" = '!!!CFUNCTION!!!'\n");
			}

			return true;
		}

		return false;
	}

	if ((unsigned int)indentLevel < maxIndentLevel)
		file.Indent(indentSpaces);
	else
		file.Print(" ");

	// If the object has a name, write it out.
	if (name)
	{
		file.Print("%s = ", name);
	}

	LuaObject key(this);
	bool ret = DumpObject(file, key, value, flags | 0xF0000000, indentLevel, maxIndentLevel);
	file.Print("\n");
	return ret;
}


/**
	Save the complete script state.
**/
bool LuaState::DumpObject(const char* filename, LuaObject& key, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	if (!key.IsString())
	{
		// Open the text file to write the script state to.
		LuaStateOutFile regFile;

		LuaStateOutFile* file = &regFile;
		if (!file->Open(filename))
			return false;

		return DumpObject(*file, key, value, flags, indentLevel, maxIndentLevel);
	}
	else
	{
		return DumpObject(filename, key.GetString(), value, flags, indentLevel, maxIndentLevel);
	}
}


/**
	Save the complete script state.
**/
bool LuaState::DumpObject(const char* filename, const char* name, LuaObject& value,
						 unsigned int flags, int indentLevel, unsigned int maxIndentLevel)
{
	// Open the text file to write the script state to.
	LuaStateOutFile regFile;

	LuaStateOutFile* file = &regFile;
	if (!file->Open(filename))
		return false;

	// Yes, this is hack-ish.

	// If the value is nil, don't write it.
	if (value.IsNil())
		return false;

	// If the variable is user data or a function...
	if (value.IsUserdata()  ||  value.IsFunction()  ||  value.IsCFunction())
	{
		// ... only write it if they requested it be written.  But only do
		// it as a comment.
		if ((flags & DUMP_WRITEALL)  &&  name)
		{
			if (value.IsUserdata())
			{
				file->Print("-- %s", name);
				file->Print(" = '!!!USERDATA!!!'\n");
			}
			else if (value.IsFunction())
			{
				lua_Debug ar;
				value.Push(this);
				lua_getinfo(*this, ">S", &ar);
//				printf("%d\n", ar.linedefined);
				file->Print("-- %s", name);
				file->Print(" = '!!!FUNCTION!!! %s %d'\n", ar.source, ar.linedefined);
			}
			else
			{
				file->Print("-- %s", name);
				file->Print(" = '!!!CFUNCTION!!!'\n");
			}

			return true;
		}

		return false;
	}

	// Indent the line the number of spaces for the current indentation level.
	const unsigned int INDENT_SIZE = 1;
	const unsigned int indentSpaces = (indentLevel == -1 ? 0 : indentLevel) * INDENT_SIZE;
	if ((unsigned int)indentLevel < maxIndentLevel)
		file->Indent(indentSpaces);
	else
		file->Print(" ");

	// If the object has a name, write it out.
	if (name)
	{
		file->Print("%s = ", name);
	}

	LuaObject key(this);
	bool ret = DumpObject(*file, key, value, flags | 0xF0000000, indentLevel, maxIndentLevel);
	file->Print("\n");
	return ret;
}


/**
	Adds [indentLevel] number of spaces to the file.
**/
void LuaStateOutFile::Indent(unsigned int indentLevel)
{
	// Write out indentation.
	char spaces[500];
	unsigned int i;
	for (i = 0; i < indentLevel; ++i)
		spaces[i] = '\t';
	spaces[i] = 0;
	Print(spaces);
}

#endif // LUAPLUS_DUMPOBJECT

} // namespace LuaPlus

extern "C" int str_format_helper (luaL_Buffer *b, lua_State *L, int arg) {
	return LuaPlus::str_format_helper(b, L, arg);
}


#if LUAPLUS_DUMPOBJECT

extern "C" void luaplus_dumptable(lua_State* L, int index)
{
	LuaPlus::LuaState* state = lua_State_to_LuaState(L);
	LuaPlus::LuaObject valueObj(state, index);
	LuaPlus::LuaStateOutString stringFile;
	state->DumpObject(stringFile, NULL, valueObj, LuaPlus::LuaState::DUMP_ALPHABETICAL | LuaPlus::LuaState::DUMP_WRITEALL, 0, -1);
	state->PushString(stringFile.GetBuffer());
}


// LuaDumpObject(file, key, value, alphabetical, indentLevel, maxIndentLevel, writeAll)
extern "C" int luaplus_ls_LuaDumpObject( lua_State* L )
{
	LuaPlus::LuaState* state = lua_State_to_LuaState( L );
	LuaPlus::LuaStateOutFile file;

	LuaPlus::LuaStack args(state);
	LuaPlus::LuaStackObject fileObj = args[1];
	if (fileObj.IsTable()  &&  state->GetTop() == 1)
	{
		LuaPlus::LuaObject valueObj(fileObj);
		LuaPlus::LuaObject nameObj;
		LuaPlus::LuaStateOutString stringFile;
		state->DumpObject(stringFile, NULL, valueObj, LuaPlus::LuaState::DUMP_ALPHABETICAL, 0, -1);
		state->PushString(stringFile.GetBuffer());
		return 1;
	}

	const char* fileName = NULL;
	if ( fileObj.IsUserdata() )
	{	
		FILE* stdioFile = (FILE *)fileObj.GetUserdata();
		file.Assign( stdioFile );
	}
	else if ( fileObj.IsString() )
	{
		fileName = fileObj.GetString();
	}

	LuaPlus::LuaObject nameObj = args[2];
	LuaPlus::LuaObject valueObj = args[3];
	LuaPlus::LuaStackObject alphabeticalObj = args[4];
	LuaPlus::LuaStackObject indentLevelObj = args[5];
	LuaPlus::LuaStackObject maxIndentLevelObj = args[6];
	LuaPlus::LuaStackObject writeAllObj = args[7];
	bool writeAll = writeAllObj.IsBoolean() ? writeAllObj.GetBoolean() : false;
	bool alphabetical = alphabeticalObj.IsBoolean() ? alphabeticalObj.GetBoolean() : true;
	unsigned int maxIndentLevel = maxIndentLevelObj.IsInteger() ? (unsigned int)maxIndentLevelObj.GetInteger() : 0xFFFFFFFF;

	unsigned int flags = (alphabetical ? LuaPlus::LuaState::DUMP_ALPHABETICAL : 0) | (writeAll ? LuaPlus::LuaState::DUMP_WRITEALL : 0);

	if (fileName)
	{
		if (strcmp(fileName, ":string") == 0)
		{
			LuaPlus::LuaStateOutString stringFile;
			state->DumpObject(stringFile, nameObj, valueObj, flags, indentLevelObj.GetInteger(), maxIndentLevel);
			state->PushString(stringFile.GetBuffer());
			return 1;
		}
		else
		{
			state->DumpObject(fileName, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
		}
	}
	else
	{
		state->DumpObject(file, nameObj, valueObj, flags, (unsigned int)indentLevelObj.GetInteger(), maxIndentLevel);
	}

	return 0;
}

#endif // LUAPLUS_DUMPOBJECT

