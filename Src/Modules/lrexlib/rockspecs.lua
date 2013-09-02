-- Rockspec data

-- Variables to be interpolated:
--
-- flavour: regex library
-- version

local flavours = {"PCRE", "POSIX", "oniguruma", "TRE", "GNU"}
local version_dashed = version:gsub ("%.", "-")

-- FIXME: When Lua 5.1 support is dropped, use an env argument with
-- loadfile instead of wrapping in a table
return {

default = {
  package = "Lrexlib-"..flavour,
  version = version.."-1",
  source = {
    url = "git://github.com/rrthomas/lrexlib.git",
    tag = "rel-"..version_dashed,
  },
  description = {
    summary = "Regular expression library binding ("..flavour.." flavour).",
    detailed = [[
Lrexlib is a regular expression library for Lua 5.1 and 5.2, which
provides bindings for several regular expression libraries.
This rock provides the ]]..flavour..[[ bindings.]],
    homepage = "http://github.com/rrthomas/lrexlib",
    license = "MIT/X11"
  },
  dependencies = {
    "lua >= 5.1"
  },
},

PCRE = {
  external_dependencies = {
    PCRE = {
      header = "pcre.h",
      library = "pcre"
    }
  },
  build = {
    type = "builtin",
    modules = {
      rex_pcre = {
        defines = {"VERSION=\""..version.."\""},
        sources = {"src/common.c", "src/pcre/lpcre.c", "src/pcre/lpcre_f.c"},
        libraries = {"pcre"},
        incdirs = {"$(PCRE_INCDIR)"},
        libdirs = {"$(PCRE_LIBDIR)"}
      }
    }
  }
},

POSIX = {
  external_dependencies = {
    POSIX = {
      header = "regex.h",
    }
  },
  build = {
    type = "builtin",
    modules = {
      rex_posix = {
        defines = {"VERSION=\""..version.."\""},
        sources = {"src/common.c", "src/posix/lposix.c"}
      }
    }
  }
},

oniguruma = {
  external_dependencies = {
    ONIG = {
      header = "oniguruma.h",
      library = "onig"
    }
  },
  build = {
    type = "builtin",
    modules = {
      rex_onig = {
        defines = {"VERSION=\""..version.."\""},
        sources = {"src/common.c", "src/oniguruma/lonig.c", "src/oniguruma/lonig_f.c"},
        libraries = {"onig"},
        incdirs = {"$(ONIG_INCDIR)"},
        libdirs = {"$(ONIG_LIBDIR)"}
      }
    }
  }
},

TRE = {
  external_dependencies = {
    TRE = {
      header = "tre/tre.h",
      library = "tre"
    }
  },
  build = {
    type = "builtin",
    modules = {
      rex_tre = {
        defines = {"VERSION=\""..version.."\""},
        sources = {"src/common.c", "src/tre/ltre.c" --[[, "src/tre/tre_w.c"]]},
        libraries = {"tre"},
        incdirs = {"$(TRE_INCDIR)"},
        libdirs = {"$(TRE_LIBDIR)"}
      }
    }
  }
},

GNU = {
  external_dependencies = {
    GNU = {
      header = "regex.h",
    }
  },
  build = {
    type = "builtin",
    modules = {
      rex_gnu = {
        defines = {"VERSION=\""..version.."\""},
        sources = {"src/common.c", "src/gnu/lgnu.c"}
      }
    }
  }
},

} -- close wrapper table
