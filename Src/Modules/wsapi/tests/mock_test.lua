
local connector = require "wsapi.mock"

function hello(wsapi_env)
  local headers = { ["Content-type"] = "text/html" }
  local function hello_text()
    coroutine.yield("hello world!")
  end
  return 200, headers, coroutine.wrap(hello_text)
end

do
  print("Test successful request")
  local app = connector.make_handler(hello)
  local response, request = app:get("/", {hello = "world"})
  assert(response.code                    == 200)
  assert(request.request_method           == "GET")
  assert(request.query_string             == "hello=world")
  assert(response.headers["Content-type"] == "text/html")
  assert(response.body                    == "hello world!")
end
