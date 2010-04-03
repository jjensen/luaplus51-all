local cosmo = require"cosmo"

values = { rank="Ace", suit="Spades" } 
template = "$rank of $suit"
result = cosmo.fill(template, values) 
assert(result== "Ace of Spades")
  
mycards = { {rank="Ace", suit="Spades"}, {rank="Queen", suit="Diamonds"}, {rank="10", suit="Hearts"} } 
template = "$do_cards[[$rank of $suit, ]]"
result = cosmo.fill(template, {do_cards = mycards})  
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, ")

result= cosmo.f(template){do_cards = mycards}
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, ")

mycards = { {"Ace", "Spades"}, {"Queen", "Diamonds"}, {"10", "Hearts"} }
result = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
                 cosmo.yield{rank=v[1], suit=v[2]}
              end
           end
        }
assert(result=="Ace of Spades, Queen of Diamonds, 10 of Hearts, ")

table.insert(mycards, {"2", "Clubs"})
template = "You have: $do_cards[[$rank of $suit]],[[, $rank of $suit]],[[, and $rank of $suit]]"
result = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
		 local template
                 if i == #mycards then -- for the last item use the third template (with "and")
                    template = 3
                 elseif i~=1 then -- use the second template for items 2...n-1
                    template = 2
                 end
                 cosmo.yield{rank=v[1], suit=v[2], _template=template}
              end
           end
        }
assert(result=="You have: Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs")
   

template = [====[You have: $do_cards[[$rank of $suit]],
[[, $rank of $suit]],
[[, and $rank of $suit]]]====]
result = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
		 local template
                 if i == #mycards then -- for the last item use the third template (with "and")
                    template = 3
                 elseif i~=1 then -- use the second template for items 2...n-1
                    template = 2
                 end
                 cosmo.yield{rank=v[1], suit=v[2], _template=template}
              end
           end
        }
assert(result=="You have: Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs")


players = {"John", "João"}
cards = {}
cards["John"] = mycards
cards["João"] = { {"Ace", "Diamonds"} }
template = "$do_players[=[$player has $do_cards[[$rank of $suit]],[[, $rank of $suit]],[[, and $rank of $suit]]\n]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          local template
                          if i == #mycards then -- for the last item use the third template (with "and")
                             template = 3
                          elseif i~=1 then -- use the second template for items 2...n-1
                             template = 2
                          end
                          cosmo.yield{rank=v[1], suit=v[2], _template=template}
                       end
                    end
                 }         
             end
          end
        }
assert(result=="John has Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs\nJoão has Ace of Diamonds\n")

template = "$do_players[=[$player$if_john[[$mark]] has $do_cards[[$rank of $suit, ]]\n]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          cosmo.yield{rank=v[1], suit=v[2]}
                       end
                    end,
                    if_john = cosmo.c(p=="John"){mark="*"}
                 }         
             end
          end
        }

assert(result=="John* has Ace of Spades, Queen of Diamonds, 10 of Hearts, 2 of Clubs, \nJoão has Ace of Diamonds, \n")

template = "$do_players[=[$do_cards[[$rank of $suit ($player), ]]]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          cosmo.yield{rank=v[1], suit=v[2]}
                       end
                    end,
                 }         
             end
          end
        }

assert(result=="Ace of Spades (John), Queen of Diamonds (John), 10 of Hearts (John), 2 of Clubs (John), Ace of Diamonds (João), ")

template = "$do_players[=[$player: $n card$if_plural[[s]] $if_needs_more[[(needs $n more)]]\n]=]"
result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    n = #cards[p],
                    if_plural = cosmo.cond(#cards[p] > 1, {}),
                    if_needs_more = cosmo.cond(#cards[p] < 3, { n = 3-#cards[p] })
                 }         
             end
          end
        }
assert(result=="John: 4 cards \nJoão: 1 card (needs 2 more)\n")

result = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    n = #cards[p],
                    if_plural = cosmo.c(#cards[p] > 1){},
                    if_needs_more = cosmo.c(#cards[p] < 3){ n = 3-#cards[p] }
                 }         
             end
          end
        }
assert(result=="John: 4 cards \nJoão: 1 card (needs 2 more)\n")

template = " $foo|bar $foo|1|baz "
result = cosmo.fill(template, { foo = { { baz = "World!" }, bar = "Hello" } })
assert(result==" Hello World! ")

template = " Hello $message{ 'World!' } "
result = cosmo.fill(template, { message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $message{ $msg } "
result = cosmo.fill(template, { msg = "World!", message = function (arg) return arg[1] end })
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.fill(template, { msg = "World!", 
		       message = function (arg) cosmo.yield{ it = arg[1] } end })
assert(result==" Hello World! ")

template = " $foo|bar $foo|1|baz "
result = cosmo.f(template){ foo = { { baz = "World!" }, bar = "Hello" } }
assert(result==" Hello World! ")

template = " Hello $message{ 'World!' } "
result = cosmo.f(template){ message = function (arg) return arg[1] end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg } "
result = cosmo.f(template){ msg = "World!", message = function (arg) return arg[1] end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.f(template){ msg = "World!", 
		       message = function (arg) cosmo.yield{ it = arg[1] } end }
assert(result==" Hello World! ")


template = " Hello $message{ $msg } "
result = cosmo.f(template){ msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.f(template){ msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end }
assert(result==" Hello World! ")

template = " Hello $message{ $msg } "
result = cosmo.fill(template, { msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end })
assert(result==" Hello World! ")

template = " Hello $message{ $msg }[[$it]] "
result = cosmo.fill(template, { msg = "World!", 
   message = function (arg, has_block) 
		if has_block then
		   cosmo.yield{ it = arg[1] }
		else
		   return arg[1] 
		end
	     end })
assert(result==" Hello World! ")

template = " $message{ greeting = 'Hello', target = 'World' } "
result = cosmo.fill(template, { message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end })
assert(result==" Hello World! ")

template = " $message{ greeting = 'Hello', target = 'World' } "
result = cosmo.f(template){ message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end }
assert(result==" Hello World! ")

template = " $message{ greeting = 'Hello', target = 'World' }[[$grt $tgt]] "
result = cosmo.fill(template, { message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end })
assert(result==" Hello World ")

template = " $message{ greeting = 'Hello', target = 'World'}[[$grt $tgt]] "
result = cosmo.f(template){ message = function(arg, has_block)
					     if has_block then
						cosmo.yield{ grt = arg.greeting, tgt = arg.target }
					     else
						return arg.greeting .. " " .. arg.target .. "!"
					     end
					  end }
assert(result==" Hello World ")

local env = {}
setmetatable(env, { __index = { text = "Hello World!" } })
template = " $show[[$text]] "
result = cosmo.fill(template, { show = function () cosmo.yield(env) end })
assert(result == " Hello World! ")
result = cosmo.f(template){ show = function () cosmo.yield(env) end }
assert(result == " Hello World! ")

template = " $map{ 1, 2, 3, 4, 5}[[$it]] "
result = cosmo.fill(template, { map = cosmo.map })
assert(result == " 12345 ")
result = cosmo.f(template){ map = cosmo.map }
assert(result == " 12345 ")

template = " $map{ 1, 2, 3, 4, 5} "
result = cosmo.fill(template, { map = cosmo.map })
assert(result == " 12345 ")
result = cosmo.f(template){ map = cosmo.map }
assert(result == " 12345 ")

template = "$inject{ msg = 'Hello', target = 'World' }[[ $msg $target! ]]"
result = cosmo.fill(template, { inject = cosmo.inject })
assert(result == " Hello World! ")
result = cosmo.f(template){ inject = cosmo.inject }
assert(result == " Hello World! ")
