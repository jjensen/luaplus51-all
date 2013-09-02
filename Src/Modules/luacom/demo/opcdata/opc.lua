
require "luacom"

opc = luacom.CreateObject("OPCDataCtrl.OPCDataCtrl.1")
assert(opc)

opc:setOPCServer("Matrikon.OPC.Simulation.1")

print(opc.OPCServer)
assert(opc:Connect())

print(opc:ReadVariable("Random.Int2"))
print(opc:ReadVariable("Random.Int2"))
print(opc:ReadVariable("Random.Int2"))
code, t1, t2 = opc:ReadMultiVariables({ Type = "array of string", Value = { "Random.Int2", "Square Waves.Real8" } })
print(code)
for k, v in pairs(t1) do
  print(k, v)
end
for k, v in pairs(t2) do
  print(k, v)
end

