package = "WSAPI-FCGI"

version = "1.6.1-1"

description = {
  summary = "Lua Web Server API FastCGI Adapter",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This
    is the rock that contains the FCGI module lfcgi.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi >= 1.6.1", "rings >= 1.3.0", "coxpcall >= 1.14" }

external_dependencies = {
  platforms = {
    unix = {
      FASTCGI = {
        header = "fcgi_stdio.h"
      }
    }
  }
}

source = {
  url = "http://www.keplerproject.org/files/wsapi-1.6.1.tar.gz"
}

build = {
  platforms = {
    unix = {
      type = "builtin",
      modules = {
        ["wsapi.fastcgi"] = "src/wsapi/fastcgi.lua",
        lfcgi = {
          sources = "src/fastcgi/lfcgi.c",
          libraries = "fcgi",
          incdirs = "$(FASTCGI_INCDIR)",
          libdirs = "$(FASTCGI_LIBDIR)"
        }
      },
      install = { bin = { "src/launcher/wsapi.fcgi" } }
    },
    windows = {
      type = "builtin",
      modules = {
        ["wsapi.fastcgi"] = "src/wsapi/fastcgi.lua",
        lfcgi = {
          sources = "src/fastcgi/lfcgi.c",
          libraries = { "libfcgi", "ws2_32" },
          incdirs = "$(FASTCGI_INCDIR)",
          libdirs = "$(FASTCGI_LIBDIR)"
        }
      },
      install = { bin = { "src/launcher/wsapi.fcgi" } }
    }
  }
}
