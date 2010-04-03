<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
 <head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
  <meta name="description" content="Cosmo: Safe Templates in Lua">
  <meta name="keywords" content="Lua, Cosmo, safe templates">
  <title>Cosmo</title>
  <style type="text/css">
    body { color:#000; background:#fff; }
    #header { width:100%;
       text-align:center;  
       border-top:solid #aaa 1px;
       border-bottom:solid #aaa 1px;
    }
    #header p { margin-left:0; }
    p { margin-left:3px; }
    pre { background-color:#ffe; white-space:pre; padding-left:3ex; border-left: 1px solid gray; margin-left: 10px}
  </style>
</head>
<body>
 <div id="header">
  <img border=0 alt="Cosmo Logo" src="cosmo.png"/>
  <p>Safe Templates in Lua</p>
  <p>
   <a href="#Overview">Overview</a> &middot;
   <a href="#Installation">Installation</a> &middot;
   <a href="#Using">Using Cosmo</a> &middot;
   <a href="#Contact">Contact Us</a> &middot;
   <a href="#License">License</a> 

  </p>
 </div>

<a name="Overview"></a> Overview
=================================================

Cosmo is a "safe templates" engine.  It allows you to fill nested
templates, providing many of the advantages of Turing-complete
template engines, without without the downside of allowing arbitrary
code in the templates.

<a name="Overview"></a>Installation
=================================================

The current version of Cosmo is 8.04.04. This is a maintenance release
for compatibility with LPEG 0.8.1.

Cosmo is installed as a rock. To install the most recent release
do `luarocks install cosmo`. The Cosmo rock is in the standard
repository. Installation on UNIX-based systems need the gcc toolchain.

<a name="Using"></a>Using Cosmo
=================================================

## Simple Form Filling

Let's start with a simple example of filling a set of scalar values
into a template: Here are a few examples of Cosmo in use:

    > values = { rank="Ace", suit="Spades" } 
    > template = "$rank of $suit"
    > require("cosmo")
    > = cosmo.fill(template, values)
    Ace of Spades

Note that the template is a string that marks where values should go.
The table provides the values.  `$rank` will get replaced by
`value.rank` ("Ace") and `$suit` will get replaced by `value.suit`
("Spades").

`cosmo.fill()` takes two parameters at once.  Cosmo also provides a
"shortcut" method `f()` which takes only one parameter - the template
- and returns a function that then takes the second parameter.  This
allows for a more compact notation:

    > = cosmo.f(template){ rank="Ace", suit="Spades" } 
    Ace of Spades

## Nested Values

You aren't restricted to scalar values; your values can be Lua tables that
you can destructure using a *$val|key1|key2|...|keyn* syntax. For example:

    > values = { cards = { { rank = "Ace" , suit = "Spades" } } }
    > template = "$cards|1|rank of $cards|1|suit"
    > = cosmo.fill(template, values)
    Ace of Spades

As you can see above, you can either use numbers or strings as keys.

## Arguments

You can also pass arguments to a selector using the syntax *$selector{ args }*.
Each argument can be a list of arguments enclosed by {}, a number, a string literal
(including long strings),
a *key = value* pair, where key is a valid Lua name and value is an argument,
a *[key] = value* pair, where key and value are both arguments, `true`, `false`, 
`nil`, or a Cosmo selector.
If the argument is a selector it is looked up in the template environment.

If you pass an argument list and the selector maps to a function then Cosmo calls
this function with the argument list as a table, and the selector expands to what
the function returns. For example:

    > values = { message = function (arg) return arg.rank .. " of "
         .. arg.suit end }
    > template = "$message{ rank = 'Ace', suit = 'Spades' }"
    > = cosmo.fill(template, values)
    Ace of Spades

## Subtemplates

Now, suppose we have not just one card, but several.  Cosmo allows us
to handle this case with "subtemplates"

    > mycards = { {rank="Ace", suit="Spades"}, {rank="Queen", suit="Diamonds"}, {rank="10", suit="Hearts"} } 
    > template = "$do_cards[[$rank of $suit, ]]"
    > = cosmo.fill(template, {do_cards = mycards})  
    Ace of Spades, Queen of Diamonds, 10 of Hearts,

The subtemplate "$rank or $suit" could be enclosed in `[[...]]`,
`[=[...]=]`, `[==[...]==]`, etc. - just like Lua's long-quoted
strings.  Again, we can use the shortcut `f()`:

    > = cosmo.f(template){do_cards = mycards}  
    Ace of Spades, Queen of Diamonds, 10 of Hearts,

## Subtemplates with Functions

If we don't have a ready table that would match the template, we can
set the value of `do_cards` to a function, which will yield a set of
values for the subtemplate each time it's called:

    > mycards = { {"Ace", "Spades"}, {"Queen", "Diamonds"}, {"10", "Hearts"} }
    > = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
                 cosmo.yield{rank=v[1], suit=v[2]}
              end
           end
        }
    Ace of Spades, Queen of Diamonds, 10 of Hearts,

You can also pass a list of arguments to this function:

    > template = "$do_cards{ true, false, true }[[$rank of $suit, ]]"
    > mycards = { {"Ace", "Spades"}, {"Queen", "Diamonds"}, {"10", "Hearts"} }
    > = cosmo.f(template){
           do_cards = function(arg)
              for i,v in ipairs(mycards) do
                 if arg[i] then cosmo.yield{rank=v[1], suit=v[2]} end
              end
           end
        }
    Ace of Spades, 10 of Hearts,

## Alternative Subtemplates

In some cases we may want to use differente templates for different
items in the list.  For example, we might want to use a different
template for the first and/or last item, or to use different templates
for odd and even numbers. We can do this by specifying several
templates, separated by a comma.  In that case, cosmo will use the
first template in the sequence, unless the table of values for the
item contains a special field `_template`, in which case this field
will be used as an index into the list of alternative templates.  For
instance, setting `_template` to 2 would tell cosmo to use the 2nd
template for this item.

    > table.insert(mycards, {"2", "Clubs"})
    > template = "You have: $do_cards[[$rank of $suit]],[[, $rank of $suit]],[[, and $rank of $suit]]"
    > = cosmo.f(template){
           do_cards = function()
              for i,v in ipairs(mycards) do
                 local t
                 if i == #mycards then -- for the last item use the third template
                    t = 3
                 elseif i~=1 then -- use the second template for items 2...n-1
                    t = 2
                 end
                 cosmo.yield{rank=v[1], suit=v[2], _template=t}
              end
           end
        }

    You have: Ace of Spades, Queen of Diamonds, 10 of Heards, and 2 of Clubs

Note that the first item is formatted without preceeding ", ", while
the last item is preceeded by an extra "and".

## Deeper Nesting

Templates and subtemplates can be nested to arbitrary depth.  For
instance, instead of formatting a set of cards, we can format a list
of sets of cards:
   
    > players = {"John", "João"}
    > cards = {}
    > cards["John"] = mycards
    > cards["João"] = { {"Ace", "Diamonds"} }
    > template = "$do_players[=[$player has $do_cards[[$rank of $suit]],
        [[, $rank of $suit]],[[, and $rank of $suit]]\n]=]"
    > = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    do_cards = function()
                       for i,v in ipairs(cards[p]) do
                          local t
                          if i == #mycards then
                             t = 3
                          elseif i~=1 then -- use the second template for items 2...n-1
                             t = 2
                          end
                          cosmo.yield{rank=v[1], suit=v[2], _template=t}
                       end
                    end
                 }         
             end
          end
        }

    John has Ace of Spades, Queen of Diamonds, 10 of Hearts, and 2 of Clubs
    João has Ace of Diamonds

## Scope

Subtemplates can see values that were set in the higher scope:

    > template = "$do_players[=[$do_cards[[$rank of $suit ($player), ]]]=]"
    > = cosmo.f(template){
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

    Ace of Spades (John), Queen of Diamonds (John), 10 of Hearts (John), 2 of Clubs (John), Ace of Diamonds (João), 

Note that in this case the field "player" is set in the table of
values that is passed to `do_players`, but is used one level deeper -
in `do_cards`.

The scoping behavior can be overriden by setting a metatable on the environment you
pass to the subtemplates.

## Conditionals

In some cases we want to format an set of values if some condition
applies.  This can be done with a function and a subtemplate by just
replacing a for-loop with an if-block.  However, since this is a
common case, cosmo provides a function for it:

    > template = "$do_players[=[$player: $n card$if_plural[[s]] $if_needs_more[[(needs $n more)]]\n]=]"
    > = cosmo.f(template){
           do_players = function()
              for i,p in ipairs(players) do
                 cosmo.yield {
                    player = p,
                    n = #cards[p],
                    if_plural = cosmo.cond(#cards[p] > 1, {}),
                    if_needs_more = cosmo.cond(#cards[p] < 3, { n = 3 - #cards[p] })
                 }         
              end
           end
        }

    John: 4 cards
    João: 1 card (needs 2 more)

Like `fill()`, `cond()` has a "shortcut" equivalent which takes only
one parameter (the template) and returns a function:

    > = cosmo.f(template){
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

    John: 4 cards
    João: 1 card (needs 2 more)

## Map and Inject

Cosmo provides two convenience functions for writing simple templates, `cosmo.map`
and `cosmo.inject`. Both functions have to be passed in a template's environment.
The `cosmo.map` function yields each of its arguments in sequence, and inject yields
its whole argument table. A simple example:

    > template = "<ol>\n$map{ 'Spades', 'Hearts', 'Clubs', 'Diamonds'}[[<li>$it</li>\n]]</ol>"
    > = cosmo.fill(template, { map = cosmo.map })
    <ol>
    <li>Spades</li>
    <li>Hearts</li>
    <li>Clubs</li>
    <li>Diamonds</li>
    </ol>
    > template = "$inject{ suit = 'Spades' }[[Ace of <b>$suit</b>]]"
    > = cosmo.fill(template, { inject = cosmo.inject })
    Ace of <b>Spades</b>

<a name="Contact"></a> Contact Us
=================================================

For more information please contact one of the authors, 
[Fabio Mascarenhas](mailto:mascarenhas_no_spam@acm.org) and [Yuri
Takhteyev](http://takhteyev.org/contact/), or write to the
[Sputnik Mailing List](http://sputnik.freewisdom.org/en/Mailing_List).

Comments are welcome! 

<a name="License"></a> License
=================================================

Cosmo is free software: it can be used for both academic and
commercial purposes at absolutely no cost.  There are no royalties or
GNU-like "copyleft" restrictions. Cosmo qualifies as Open Source
software.  Its licenses are compatible with GPL. The legal details are
below.

The spirit of the license is that you are free to use Cosmo for any
purpose at no cost without having to ask us. The only requirement is
that if you do use Cosmo, then you should give us credit by including
the appropriate copyright notice somewhere in your product or its
documentation.

The original Cosmo library is designed and implemented by Yuri Takhteyev, with
much feedback and inspiration by André Carregal. This version is a reimplementation
by Fabio Mascarenhas, with aditional features. The implementations
are not derived from licensed software.

Copyright © 2008 Fabio Mascarenhas.
Copyright © 2007-2008 Yuri Takhteyev.

---------------------------------

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</body>
</html>
