///////////////////////////////////////////////////////////////////////////////
// This source file is part of the LuaPlus source distribution and is Copyright
// 2001-2011 by Joshua C. Jensen (jjensen@workspacewhiz.com).
//
// The latest version may be obtained from http://luaplus.org/.
//
// The code presented in this file may be used in any environment it is
// acceptable to use Lua.
///////////////////////////////////////////////////////////////////////////////
#ifndef LUAPLUS__LUAHELPER_OBJECT_H
#define LUAPLUS__LUAHELPER_OBJECT_H

#include "LuaPlusInternal.h"

///////////////////////////////////////////////////////////////////////////////
// namespace LuaPlus
///////////////////////////////////////////////////////////////////////////////
namespace LuaPlus
{

/**
**/
namespace LuaHelper
{
	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	inline bool GetBoolean( LuaObject& obj, int key, bool require = true, bool defaultValue = false );
	inline bool GetBoolean( LuaObject& obj, const char* key, bool require = true, bool defaultValue = false );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	inline int GetInteger( LuaObject& obj, int key, bool require = true, int defaultValue = -1 );
	inline int GetInteger( LuaObject& obj, const char* key, bool require = true, int defaultValue = -1 );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	inline float GetFloat( LuaObject& obj, int key, bool require = true, float defaultValue = -1.0f );
	inline float GetFloat( LuaObject& obj, const char* key, bool require = true, float defaultValue = -1.0f );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	inline void* GetLightUserData( LuaObject& obj, int key, bool require = true, void* defaultValue = NULL );
	inline void* GetLightUserData( LuaObject& obj, const char* key, bool require = true, void* defaultValue = NULL );


	/**
		Attempts retrieval of the value from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\param defaultValue The default value to be returned if require is
			not true and the key doesn't exist or the found value is not
			of the right type.
		\return Returns the value found or the defaultValue.
	**/
	inline const char* GetString( LuaObject& obj, int key, bool require = true, const char* defaultValue = "" );
	inline const char* GetString( LuaObject& obj, const char* key, bool require = true, const char* defaultValue = "" );


	/**
		Attempts retrieval of the obj from the passed in LuaObject.

		\param obj The LuaObject representing a table to attempt retrieval
			of the value from.
		\param key The lookup key for the value.  The key may be of number,
			string, or LuaObject type.
		\param require If set to true, the key must exist in the table and
			must be of the requested type.  If it doesn't exist or isn't of
			the type, an assertion fires.  Otherwise, defaultValue is 
			returned.
		\return Returns the object found.
	**/
	inline LuaObject GetTable( LuaObject& obj, int key, bool require = true );
	inline LuaObject GetTable( LuaObject& obj, const char* key, bool require = true );

	inline void MergeObjects( LuaObject& mergeTo, LuaObject& mergeFrom, bool replaceDuplicates );

inline bool GetBoolean( LuaObject& obj, int key, bool require, bool defaultValue ) {
	LuaObject boolObj = obj[ key ];
	if ( !boolObj.IsBoolean() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return boolObj.GetBoolean();
}


inline bool GetBoolean( LuaObject& obj, const char* key, bool require, bool defaultValue ) {
	LuaObject boolObj = obj[ key ];
	if ( !boolObj.IsBoolean() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return boolObj.GetBoolean();
}


inline int GetInteger( LuaObject& obj, int key, bool require, int defaultValue ) {
	LuaObject intObj = obj[ key ];
	if ( !intObj.IsInteger() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return intObj.GetInteger();
}


inline int GetInteger( LuaObject& obj, const char* key, bool require, int defaultValue ) {
	LuaObject intObj = obj[ key ];
	if ( !intObj.IsInteger() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return intObj.GetInteger();
}


inline float GetFloat( LuaObject& obj, int key, bool require, float defaultValue ) {
	LuaObject floatObj = obj[ key ];
	if ( !floatObj.IsNumber() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return (float)floatObj.GetNumber();
}


inline float GetFloat( LuaObject& obj, const char* key, bool require, float defaultValue ) {
	LuaObject floatObj = obj[ key ];
	if ( !floatObj.IsNumber() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return (float)floatObj.GetNumber();
}


inline void* GetLightUserData( LuaObject& obj, int key, bool require, void* defaultValue ) {
	LuaObject outObj = obj[ key ];
	if ( !outObj.IsLightUserData() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return outObj.GetLightUserData();
}


inline void* GetLightUserData( LuaObject& obj, const char* key, bool require, void* defaultValue ) {
	LuaObject outObj = obj[ key ];
	if ( !outObj.IsLightUserData() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return outObj.GetLightUserData();
}


inline const char* GetString( LuaObject& obj, int key, bool require, const char* defaultValue ) {
	LuaObject stringObj = obj[ key ];
	if ( !stringObj.IsString() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return stringObj.GetString();
}


inline const char* GetString( LuaObject& obj, const char* key, bool require, const char* defaultValue ) {
	LuaObject stringObj = obj[ key ];
	if ( !stringObj.IsString() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
		return defaultValue;
	}
	return stringObj.GetString();
}


inline LuaObject GetTable( LuaObject& obj, int key, bool require ) {
	LuaObject tableObj = obj[ key ];
	if ( !tableObj.IsTable() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
	}
	return tableObj;
}


inline LuaObject GetTable( LuaObject& obj, const char* key, bool require ) {
	LuaObject tableObj = obj[ key ];
	if ( !tableObj.IsTable() ) {
		if ( require ) {
			luaplus_assert( 0 );
		}
	}
	return tableObj;
}


inline void MergeObjects(LuaObject& mergeTo, LuaObject& mergeFrom, bool replaceDuplicates)
{
	if (mergeTo.GetState() == mergeFrom.GetState())
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject toNodeKeyObj = mergeTo[it.GetKey()];
			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				mergeTo.Set(it.GetKey(), it.GetValue());
			}
		}
	}
	else
	{
		for (LuaTableIterator it(mergeFrom); it; ++it)
		{
			LuaObject obj;
			switch (it.GetKey().Type())
			{
				case LUA_TBOOLEAN:	obj.Assign(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
				case LUA_TNUMBER:	obj.Assign(mergeTo.GetState(), it.GetKey().GetNumber());			break;
				case LUA_TSTRING:	obj.Assign(mergeTo.GetState(), it.GetKey().GetString());			break;
#if LUA_WIDESTRING
				case LUA_TWSTRING:	obj.Assign(mergeTo.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
			}

			LuaObject toNodeKeyObj = mergeTo[obj];

			if (it.GetValue().IsTable())
			{
				if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
				{
					toNodeKeyObj = mergeTo.CreateTable(it.GetKey());
				}
				MergeObjects(toNodeKeyObj, it.GetValue(), replaceDuplicates);
			}
			else if (toNodeKeyObj.IsNil()  ||  replaceDuplicates)
			{
				LuaObject toKeyObj;
				switch (it.GetKey().Type())
				{
					case LUA_TBOOLEAN:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetBoolean());		break;
					case LUA_TNUMBER:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetNumber());			break;
					case LUA_TSTRING:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetString());			break;
#if LUA_WIDESTRING
					case LUA_TWSTRING:	toKeyObj.Assign(mergeTo.GetState(), it.GetKey().GetWString());		break;
#endif /* LUA_WIDESTRING */
				}

				switch (it.GetValue().Type())
				{
					case LUA_TBOOLEAN:	mergeTo.Set(toKeyObj, it.GetValue().GetBoolean());	break;
					case LUA_TNUMBER:	mergeTo.Set(toKeyObj, it.GetValue().GetNumber());		break;
					case LUA_TSTRING:	mergeTo.Set(toKeyObj, it.GetValue().GetString());		break;
#if LUA_WIDESTRING
					case LUA_TWSTRING:	mergeTo.Set(toKeyObj, it.GetValue().GetWString());	break;
#endif /* LUA_WIDESTRING */
				}
			}
		}
	}
}

} // namespace LuaHelper


} // namespace LuaPlus


#endif // LUAPLUS__LUAHELPER_OBJECT_H
