local db = 'blog'

require "luasql.mysql"
require "orbit.model"

local env = luasql.mysql()
local conn = env:connect(db, "root", "password")

local mapper = orbit.model.new("toycms_", conn, "mysql")



-- Table post

local t = mapper:new('post')

-- Record 1

local rec = {
 ["id"] = 1,
 ["user_id"] = 1,
 ["published"] = true,
 ["body"] = "## General FAQ\
\
### What is Kepler?\
\
Kepler is a a set of components for Lua that make up a powerful Web development platform. It is also the name of the project that is developing the Kepler platform.\
\
### Why \"Kepler\"?\
\
Johannes Kepler was the astronomer that first explained that the tides are caused by the Moon. \"Lua\" means Moon in Portuguese, so the name \"Kepler\" tries to hint that some new tides may soon be caused by Lua... :o)\
\
### What is Lua?\
\
Lua is a programming language that offers a very impressive set of features while keeping everything fast, small and portable. The Kepler Platform uses these features to offer a faster, smaller and more portable way to develop Web applications.  See [[Lua]] for more information.\
\
### What is a Web application?\
\
Web applications (also known as Web apps) are programs that are used through a Web browser. Every time you search for something with Google, read mail with Hotmail or browse Amazon you are using a Web application. Some other examples would be discussion forums, a blog or a weather site.\
\
### What is a Web development platform?\
\
Web applications can be developed in different ways, from a hands-on approach to a very structured one. A Web development platform offers the developer a number of features that make the development of Web applications a lot easier. Instead of developing the application from scratch, the developer can benefit from the building blocks of the Web platform.\
\
### Why build/use another Web development platform?\
\
There are a number of great Web development platforms out there but none balances power, size and flexibility quite like Kepler does.\
\
### Is Kepler better than PHP/Java/.Net/...?\
\
That depends on what is your goal. Those platforms are surely good solutions for Web Development but sometimes they turn out to be too big, too rigid or just too unportable for the job. That's precisely where Kepler shines.\
\
### What does Kepler offer for the developer?\
\
Kepler offers a Core plus a small but powerful combination of components. You have components for SQL Database access, XML parsing, Logging, ZIP manipulation and some others.\
\
### What is the Kepler Core?\
\
The Kepler Core is the minimum set of components of Kepler:\
\
- [[CGILua]] for page generation\
- [[LuaSocket]] for TCP/UDP sockets \
\
### What is the Kepler Platform?\
\
The Kepler Platform includes the Kepler Core plus a defined set of components:\
\
- [[LuaExpat]] for XML parsing\
- [[LuaFileSystem]]\
- [[LuaLogging]]\
- [[LuaSQL]] for database access\
- [[LuaZIP]] for ZIP manipulation \
\
### Why separate the Kepler Core from the Kepler Platform?\
\
If you don't need all the Kepler Platform components or prefer to add your own components, you can simply get only the Kepler Core as a starting point. But if you choose to develop for the Kepler Platform you can benefit from some important points:\
\
- you will be able to easily upgrade your development platform as Kepler continues to evolve.\
- you will be using the same set of components as other Kepler Platform developers, making it easier to exchange ideas and experience.\
- you can be assured of the portability of your Web application for other environments, as long as those environments also run the Kepler Platform. \
\
### Do I need to use the Kepler Platform to use the Kepler Project components?\
\
Not at all! The components developed by the Kepler Project can be used in any Lua based system. You can compile them from the source files or use the binary versions, both available free of charge on LuaForge.\
\
### What about the licensing and pricing models?\
\
Kepler and Lua are free software: they can be used for both academic and commercial purposes at absolutely no cost. There are no royalties or GNU-like \"copyleft\" restrictions. Kepler and Lua qualifies as Open Source software. Their licenses are compatible with GPL. Kepler is not in the public domain and the Kepler Project keeps its copyright.  (See also: [[License]].)\
\
### What is CGILua?\
\
CGILua is the main component of the Kepler Core. It is a tool for creating dynamic Web pages and manipulating input data from Web forms. Among other functions, CGILua is the component responsible for the user interface programming side of your Web application, while the remaining ones handle the logic and data of your Web application.\
\
One of the big advantages of CGILua is its abstraction of the underlying Web server. You can develop a CGILua application for one Web server and run it on any other Web server that supports CGILua.  See [[CGILua]] for more details.\
\
### Do I have to use Kepler to use CGILua?\
\
No, although it is probably a lot easier to get Kepler and simply start using CGILua than to get CGILua's source and build a launcher for it from scratch. You may also benefit from the fact that Kepler includes lot of ready to use CGILua launchers so you have more choices of Web servers.\
\
### What are CGILua launchers?\
\
A CGILua launcher is the mechanism that allows a Web server to execute and communicate with CGILua and its Web applications.\
\
### Which CGILua launchers are available?\
\
Kepler currently offers the following set of CGILua launchers:\
\
- CGI\
- FastCGI\
- mod_lua (for Apache)\
- ISAPI (for Microsoft IIS)\
- [[Xavante]] (a Lua Web server that supports CGILua natively) \
\
You can choose your launcher based on its size, ease of use, portability or performance. You can start using a simpler launcher and then migrate to a more advanced one without any changes to your application.\
\
With this flexibility you can for example start your development locally on your Windows system running CGI and then move to a Linux server running mod_lua or even to a mobile device running Xavante.\
\
### What if my Web server is not supported?\
\
If your target Web server does not offer any of the existent connection methods or if you would prefer to use a different connection method, you have the option of creating a CGILua launcher for the target Web server.\
\
### How can I create a new CGILua launcher?\
\
A CGILua launcher implements SAPI, the Server API. SAPI consists in a set of functions that once implemented for a specific Web server architecture allows the execution of CGILua and its Web applications on it.\
\
### How ready to use is Kepler?\
\
Kepler development is an ongoing process, and you can check the latest release at the Download page. Instructions for installation on Unix and Windows can be found at the Documentation page.\
\
You can also check the [[Status]] page for the incoming releases.\
\
### Who is already using Kepler?\
\
Kepler is already being used by PUC-Rio and Hands on professional applications.\
\
### Is there a mailing list for Kepler?\
\
Yes! Kepler questions can be posted on the Kepler Project [[Mailing List]].\
\
### How can I help?\
\
There are a lot of ways to help the project and the team.\
\
One way is to use Kepler and provide some feedback. If you want to follow more closely, you can join the Kepler Project list or the Kepler forums on LuaForge.\
\
You can also help developing and debugging the existing modules, as much as helping document the platform and its modules.  Please go to the [[Developers]] section for more information for that.\
\
Another way to help would by buying something from Amazon through the PiL links on LuaForge and the Kepler sites. Doing that you'll be helping gather resources for the Kepler team.\
\
For every product (not just PiL) bought after entering Amazon through the links we get from 2% to 5% of the product price as Amazon credits. Those credits are used to buy books for the team, so we can stay sharp and deliver the goods. :o)\
\
For those interested in helping us this way, just remember that Amazon only considers products added to the cart after you enter Amazon through the Kepler links. Anything in the cart that was added during a different visit to the store will not count for us (though it may count for another Amazon partner). \
\
",
 ["published_at"] = 1183750080,
 ["image"] = "",
 ["external_url"] = "",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["n_comments"] = 1,
 ["abstract"] = "",
 ["title"] = "FAQ"}
rec = t:new(rec)
rec:save(true)

-- Record 2

local rec = {
 ["id"] = 2,
 ["user_id"] = 1,
 ["published"] = true,
 ["body"] = "## What is Kepler?\
\
**The Kepler project aims to collaboratively create an extremely portable Web development platform based on the Lua programming language, offering both flexibility and conceptual simplicity.**\
\
Kepler is an open source platform for developing Web applications in [[Lua]]. Lua is a programming language that offers a very impressive set of features while keeping everything fast, small and portable.  Kepler is implemented as a set of Lua components and offers the same advantages as Lua: it is simple, extremely portable, light, extensible and offers a very flexible licence. It allows the use of XHTML, SQL, XML, Zip and other standards. There are a number of great Web development platforms out there but none balances power, size and flexibility quite like Kepler does.\
\
The Lua community is constantly contributing with more modules that can be used with Kepler, most of those modules are catalogued on LuaForge and new ones keep coming.\
\
",
 ["published_at"] = 1183750200,
 ["image"] = "",
 ["external_url"] = "",
 ["comment_status"] = "closed",
 ["section_id"] = 2,
 ["n_comments"] = 1,
 ["abstract"] = "",
 ["title"] = "About"}
rec = t:new(rec)
rec:save(true)

-- Record 3

local rec = {
 ["id"] = 3,
 ["user_id"] = 1,
 ["published"] = true,
 ["body"] = "",
 ["published_at"] = 1183752420,
 ["image"] = "",
 ["external_url"] = "http://slashdot.org",
 ["comment_status"] = "closed",
 ["section_id"] = 3,
 ["n_comments"] = 1,
 ["abstract"] = "",
 ["title"] = "Slashdot"}
rec = t:new(rec)
rec:save(true)

-- Record 4

local rec = {
 ["id"] = 4,
 ["user_id"] = 1,
 ["published"] = true,
 ["body"] = "",
 ["published_at"] = 1183752420,
 ["image"] = "",
 ["external_url"] = "http://news.google.com",
 ["comment_status"] = "closed",
 ["section_id"] = 3,
 ["n_comments"] = 1,
 ["abstract"] = "",
 ["title"] = "Google News"}
rec = t:new(rec)
rec:save(true)

-- Record 5

local rec = {
 ["id"] = 5,
 ["user_id"] = 1,
 ["published"] = true,
 ["body"] = "",
 ["published_at"] = 1183752480,
 ["image"] = "",
 ["external_url"] = "http://www.wikipedia.org",
 ["comment_status"] = "closed",
 ["section_id"] = 3,
 ["n_comments"] = 1,
 ["abstract"] = "",
 ["title"] = "Wikipedia"}
rec = t:new(rec)
rec:save(true)

-- Record 6

local rec = {
 ["id"] = 6,
 ["user_id"] = 1,
 ["published"] = true,
 ["body"] = "Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
",
 ["published_at"] = 1096406940,
 ["image"] = "",
 ["external_url"] = "",
 ["comment_status"] = "moderated",
 ["section_id"] = 1,
 ["n_comments"] = 0,
 ["abstract"] = "",
 ["title"] = "Order Now And You Also Get A Attack Nose"}
rec = t:new(rec)
rec:save(true)

-- Record 7

local rec = {
 ["id"] = 7,
 ["body"] = "\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["published_at"] = 1096941480,
 ["title"] = "The Care And Feeding Of Your Sleeping Bicycle",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 8

local rec = {
 ["id"] = 8,
 ["body"] = "\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["published_at"] = 1097257260,
 ["title"] = "Now Anybody Can Make President",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 2}
rec = t:new(rec)
rec:save(true)

-- Record 9

local rec = {
 ["id"] = 9,
 ["body"] = "\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["published_at"] = 1097787720,
 ["title"] = "'Star Wars' As Written By A Princess",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 10

local rec = {
 ["id"] = 10,
 ["body"] = "\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["published_at"] = 1098112500,
 ["title"] = "What I Learned From An Elephant",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 11

local rec = {
 ["id"] = 11,
 ["body"] = "\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
",
 ["published_at"] = 1099335240,
 ["title"] = "Today, The World - Tomorrow, The Mixed-Up Dice",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 2}
rec = t:new(rec)
rec:save(true)

-- Record 12

local rec = {
 ["id"] = 12,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
",
 ["published_at"] = 1100140320,
 ["title"] = "The Funniest Joke About A Grandmother's Tree",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 2}
rec = t:new(rec)
rec:save(true)

-- Record 13

local rec = {
 ["id"] = 13,
 ["body"] = "\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["published_at"] = 1101168000,
 ["title"] = "Dr. Jekyll And Mr. Bear",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 2}
rec = t:new(rec)
rec:save(true)

-- Record 14

local rec = {
 ["id"] = 14,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["published_at"] = 1102720200,
 ["title"] = "Christmas Shopping For A Racing Ark",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 3}
rec = t:new(rec)
rec:save(true)

-- Record 15

local rec = {
 ["id"] = 15,
 ["body"] = "\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
",
 ["published_at"] = 1102968180,
 ["title"] = "Once Upon A Guardian Dinosaur",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 16

local rec = {
 ["id"] = 16,
 ["body"] = "\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["published_at"] = 1103047500,
 ["title"] = "The Mystery Of Lego Pirate",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 17

local rec = {
 ["id"] = 17,
 ["body"] = "\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
",
 ["published_at"] = 1104870420,
 ["title"] = "Thomas Edison Invents The Guardian Rollercoaster",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 18

local rec = {
 ["id"] = 18,
 ["body"] = "\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["published_at"] = 1104872820,
 ["title"] = "Today, The World - Tomorrow, The Complicated Moonlight",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 19

local rec = {
 ["id"] = 19,
 ["body"] = "\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["published_at"] = 1107454020,
 ["title"] = "'Star Wars' As Written By A Scary Bat",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 20

local rec = {
 ["id"] = 20,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
",
 ["published_at"] = 1107655200,
 ["title"] = "Anatomy Of A Funny Day",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 21

local rec = {
 ["id"] = 21,
 ["body"] = "\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["published_at"] = 1109017860,
 ["title"] = "On The Trail Of The Electric Desk",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 22

local rec = {
 ["id"] = 22,
 ["body"] = "\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
",
 ["published_at"] = 1112321220,
 ["title"] = "My Coach Is A New, Improved Bear",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 23

local rec = {
 ["id"] = 23,
 ["body"] = "\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["published_at"] = 1115832420,
 ["title"] = "My Son, The Lost Banana",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 24

local rec = {
 ["id"] = 24,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["published_at"] = 1116263400,
 ["title"] = "The Olympic Competition Won By An Automatic Monkey",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 25

local rec = {
 ["id"] = 25,
 ["body"] = "\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
",
 ["published_at"] = 1116268920,
 ["title"] = "The Olympic Competition Won By An Purple Bicycle",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 26

local rec = {
 ["id"] = 26,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["published_at"] = 1116340140,
 ["title"] = "Marco Polo Discovers The Complicated Spoon",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 27

local rec = {
 ["id"] = 27,
 ["body"] = "\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
",
 ["published_at"] = 1116439080,
 ["title"] = "The Mystery Of Horse",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 28

local rec = {
 ["id"] = 28,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
",
 ["published_at"] = 1116733560,
 ["title"] = "Now Anybody Can Make Miniature Nose",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 29

local rec = {
 ["id"] = 29,
 ["body"] = "\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
",
 ["published_at"] = 1116793620,
 ["title"] = "My Daughter, The Spoon",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 30

local rec = {
 ["id"] = 30,
 ["body"] = "\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["published_at"] = 1116848940,
 ["title"] = "Dental Surgery On A Desert Elephant",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 31

local rec = {
 ["id"] = 31,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["published_at"] = 1117480740,
 ["title"] = "On The Trail Of The Automatic Giant",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 32

local rec = {
 ["id"] = 32,
 ["body"] = "\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
",
 ["published_at"] = 1117649280,
 ["title"] = "What I Learned From An Football",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 33

local rec = {
 ["id"] = 33,
 ["body"] = "\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
",
 ["published_at"] = 1117716960,
 ["title"] = "Now Anybody Can Make Attack Wolf",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 34

local rec = {
 ["id"] = 34,
 ["body"] = "\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
",
 ["published_at"] = 1118028360,
 ["title"] = "Avast, Me Invisible Twin Fish",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 2}
rec = t:new(rec)
rec:save(true)

-- Record 35

local rec = {
 ["id"] = 35,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
",
 ["published_at"] = 1118244480,
 ["title"] = "Around The World With A Blustery Banana",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 36

local rec = {
 ["id"] = 36,
 ["body"] = "\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["published_at"] = 1119411720,
 ["title"] = "Mr. McMullet, The Flying Tree",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 37

local rec = {
 ["id"] = 37,
 ["body"] = "\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
",
 ["published_at"] = 1119571800,
 ["title"] = "Visit Fun World And See The Rare Recycled Clown",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 38

local rec = {
 ["id"] = 38,
 ["body"] = "\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
",
 ["published_at"] = 1119893580,
 ["title"] = "Wyoming Jones And The Secret Twin Canadian Bat",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 39

local rec = {
 ["id"] = 39,
 ["body"] = "\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["published_at"] = 1121213220,
 ["title"] = "Wyoming Jones And The Clown",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 40

local rec = {
 ["id"] = 40,
 ["body"] = "\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
",
 ["published_at"] = 1125083340,
 ["title"] = "Marco Polo Discovers The Accountant",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 3}
rec = t:new(rec)
rec:save(true)

-- Record 41

local rec = {
 ["id"] = 41,
 ["body"] = "\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["published_at"] = 1128714240,
 ["title"] = "Playing Poker With A Tree",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 42

local rec = {
 ["id"] = 42,
 ["body"] = "\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
",
 ["published_at"] = 1144002600,
 ["title"] = "Barney And The Rollercoaster",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 43

local rec = {
 ["id"] = 43,
 ["body"] = "\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["published_at"] = 1144087620,
 ["title"] = "Today, The World - Tomorrow, The Purple Money",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 44

local rec = {
 ["id"] = 44,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
",
 ["published_at"] = 1144204980,
 ["title"] = "I'm My Own Desert Friend",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 45

local rec = {
 ["id"] = 45,
 ["body"] = "\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
",
 ["published_at"] = 1144371120,
 ["title"] = "I Have To Write About My Blustery Funny Nose",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 46

local rec = {
 ["id"] = 46,
 ["body"] = "\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
",
 ["published_at"] = 1144445280,
 ["title"] = "Once Upon A Complicated Duck",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 47

local rec = {
 ["id"] = 47,
 ["body"] = "\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
",
 ["published_at"] = 1144591560,
 ["title"] = "On The Trail Of The Lost Chicken",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 48

local rec = {
 ["id"] = 48,
 ["body"] = "\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
",
 ["published_at"] = 1145244840,
 ["title"] = "Way Out West With The Green Giant",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 49

local rec = {
 ["id"] = 49,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
",
 ["published_at"] = 1145394900,
 ["title"] = "No Man Is A Monkey",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 50

local rec = {
 ["id"] = 50,
 ["body"] = "\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["published_at"] = 1145988720,
 ["title"] = "Where To Meet An Impossible Friend",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 51

local rec = {
 ["id"] = 51,
 ["body"] = "\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["published_at"] = 1146700800,
 ["title"] = "Barney And The Hungry Desk",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 5}
rec = t:new(rec)
rec:save(true)

-- Record 52

local rec = {
 ["id"] = 52,
 ["body"] = "\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
",
 ["published_at"] = 1147146240,
 ["title"] = "Way Out West With The Furry Super Wolf",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 1}
rec = t:new(rec)
rec:save(true)

-- Record 53

local rec = {
 ["id"] = 53,
 ["body"] = "\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
",
 ["published_at"] = 1147232220,
 ["title"] = "The Mystery Of Lego Chicken",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 5}
rec = t:new(rec)
rec:save(true)

-- Record 54

local rec = {
 ["id"] = 54,
 ["body"] = "\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
",
 ["published_at"] = 1149614880,
 ["title"] = "I Rode Friendly Grandmother's Mixed-Up Chocolate",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 4}
rec = t:new(rec)
rec:save(true)

-- Record 55

local rec = {
 ["id"] = 55,
 ["body"] = "\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
",
 ["published_at"] = 1149863280,
 ["title"] = "Christmas Shopping For A New, Improved Ark",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 56

local rec = {
 ["id"] = 56,
 ["body"] = "\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["published_at"] = 1150032780,
 ["title"] = "The Funniest Joke About A Scary Ark",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Record 57

local rec = {
 ["id"] = 57,
 ["body"] = "\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
",
 ["published_at"] = 1153248060,
 ["title"] = "Where To Meet An Chicken",
 ["comment_status"] = "unmoderated",
 ["section_id"] = 1,
 ["published"] = true,
 ["user_id"] = 1,
 ["n_comments"] = 0}
rec = t:new(rec)
rec:save(true)

-- Table comment

local t = mapper:new('comment')

-- Record 1

local rec = {
 ["post_id"] = 54,
 ["approved"] = true,
 ["body"] = "\
One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, it's a pretty story. Sharing everything with fun, that's the way to be. One for all and all for one, Muskehounds are always ready. One for all and all for one, helping everybody. One for all and all for one, can sound pretty corny. If you've got a problem chum, think how it could be.\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
",
 ["url"] = "",
 ["id"] = 1,
 ["author"] = "John Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 2

local rec = {
 ["post_id"] = 53,
 ["approved"] = true,
 ["body"] = "\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 3,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 3

local rec = {
 ["post_id"] = 53,
 ["approved"] = true,
 ["body"] = "\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
",
 ["url"] = "",
 ["id"] = 4,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 4

local rec = {
 ["post_id"] = 53,
 ["approved"] = true,
 ["body"] = "\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 5,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 5

local rec = {
 ["post_id"] = 53,
 ["approved"] = true,
 ["body"] = "\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
",
 ["url"] = "",
 ["id"] = 6,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 6

local rec = {
 ["post_id"] = 52,
 ["approved"] = true,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
",
 ["url"] = "",
 ["id"] = 7,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 7

local rec = {
 ["post_id"] = 51,
 ["approved"] = true,
 ["body"] = "\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
",
 ["url"] = "",
 ["id"] = 8,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 8

local rec = {
 ["post_id"] = 51,
 ["approved"] = true,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
",
 ["url"] = "",
 ["id"] = 9,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 9

local rec = {
 ["post_id"] = 51,
 ["approved"] = true,
 ["body"] = "\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
",
 ["url"] = "",
 ["id"] = 10,
 ["author"] = "John Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 10

local rec = {
 ["post_id"] = 51,
 ["approved"] = true,
 ["body"] = "\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["url"] = "",
 ["id"] = 11,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 11

local rec = {
 ["post_id"] = 51,
 ["approved"] = true,
 ["body"] = "\
Ten years ago a crack commando unit was sent to prison by a military court for a crime they didn't commit. These men promptly escaped from a maximum security stockade to the Los Angeles underground.  Today, still wanted by the government, they survive as soldiers of fortune.  If you have a problem and no one else can help, and if you can find them, maybe you can hire the A-team.\
\
",
 ["url"] = "",
 ["id"] = 12,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 12

local rec = {
 ["post_id"] = 41,
 ["approved"] = true,
 ["body"] = "\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
",
 ["url"] = "",
 ["id"] = 13,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 13

local rec = {
 ["post_id"] = 40,
 ["approved"] = true,
 ["body"] = "\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["url"] = "",
 ["id"] = 14,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 14

local rec = {
 ["post_id"] = 40,
 ["approved"] = true,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 15,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = "mascarenhas@acm.org"}
rec = t:new(rec)
rec:save(true)

-- Record 15

local rec = {
 ["post_id"] = 40,
 ["approved"] = true,
 ["body"] = "\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["url"] = "",
 ["id"] = 16,
 ["author"] = "John Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 16

local rec = {
 ["post_id"] = 39,
 ["approved"] = true,
 ["body"] = "\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 17,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = "mascarenhas@acm.org"}
rec = t:new(rec)
rec:save(true)

-- Record 17

local rec = {
 ["post_id"] = 34,
 ["approved"] = true,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 18,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = "mascarenhas@acm.org"}
rec = t:new(rec)
rec:save(true)

-- Record 18

local rec = {
 ["post_id"] = 34,
 ["approved"] = true,
 ["body"] = "\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 19,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 19

local rec = {
 ["post_id"] = 33,
 ["approved"] = true,
 ["body"] = "\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
",
 ["url"] = "",
 ["id"] = 20,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 20

local rec = {
 ["post_id"] = 20,
 ["approved"] = true,
 ["body"] = "\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 21,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = "mascarenhas@acm.org"}
rec = t:new(rec)
rec:save(true)

-- Record 21

local rec = {
 ["post_id"] = 17,
 ["approved"] = true,
 ["body"] = "\
Knight Rider, a shadowy flight into the dangerous world of a man who does not exist. Michael Knight, a young loner on a crusade to champion the cause of the innocent, the helpless in a world of criminals who operate above the law.\
\
",
 ["url"] = "",
 ["id"] = 22,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 22

local rec = {
 ["post_id"] = 16,
 ["approved"] = true,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["url"] = "",
 ["id"] = 23,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 23

local rec = {
 ["post_id"] = 15,
 ["approved"] = true,
 ["body"] = "\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["url"] = "",
 ["id"] = 24,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 24

local rec = {
 ["post_id"] = 14,
 ["approved"] = true,
 ["body"] = "\
Hey there where ya goin', not exactly knowin', who says you have to call just one place home. He's goin' everywhere, B.J. McKay and his best friend Bear. He just keeps on movin', ladies keep improvin', every day is better than the last. New dreams and better scenes, and best of all I don't pay property tax. Rollin' down to Dallas, who's providin' my palace, off to New Orleans or who knows where. Places new and ladies, too, I'm B.J. McKay and this is my best friend Bear.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["url"] = "",
 ["id"] = 25,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 25

local rec = {
 ["post_id"] = 14,
 ["approved"] = true,
 ["body"] = "\
Just the good ol' boys, never meanin' no harm. Beats all you've ever saw, been in trouble with the law since the day they was born. Straight'nin' the curve, flat'nin' the hills. Someday the mountain might get 'em, but the law never will. Makin' their way, the only way they know how, that's just a little bit more than the law will allow. Just good ol' boys, wouldn't change if they could, fightin' the system like a true modern day Robin Hood.\
\
Top Cat! The most effectual Top Cat! Who's intellectual close friends get to call him T.C., providing it's with dignity. Top Cat! The indisputable leader of the gang. He's the boss, he's a pip, he's the championship. He's the most tip top, Top Cat.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 26,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 26

local rec = {
 ["post_id"] = 14,
 ["approved"] = true,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
",
 ["url"] = "",
 ["id"] = 27,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 27

local rec = {
 ["post_id"] = 13,
 ["approved"] = true,
 ["body"] = "\
80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.\
\
",
 ["url"] = "",
 ["id"] = 28,
 ["author"] = "John Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 28

local rec = {
 ["post_id"] = 13,
 ["approved"] = true,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["url"] = "",
 ["id"] = 29,
 ["author"] = "Curly",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 29

local rec = {
 ["post_id"] = 12,
 ["approved"] = true,
 ["body"] = "\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 30,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 30

local rec = {
 ["post_id"] = 12,
 ["approved"] = true,
 ["body"] = "\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["url"] = "",
 ["id"] = 31,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 31

local rec = {
 ["post_id"] = 11,
 ["approved"] = true,
 ["body"] = "\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
",
 ["url"] = "",
 ["id"] = 32,
 ["author"] = "John Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 32

local rec = {
 ["post_id"] = 11,
 ["approved"] = true,
 ["body"] = "\
Ulysses, Ulysses - Soaring through all the galaxies. In search of Earth, flying in to the night. Ulysses, Ulysses - Fighting evil and tyranny, with all his power, and with all of his might. Ulysses - no-one else can do the things you do. Ulysses - like a bolt of thunder from the blue. Ulysses - always fighting all the evil forces bringing peace and justice to all.\
\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["url"] = "",
 ["id"] = 33,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 33

local rec = {
 ["post_id"] = 10,
 ["approved"] = true,
 ["body"] = "\
Children of the sun, see your time has just begun, searching for your ways, through adventures every day. Every day and night, with the condor in flight, with all your friends in tow, you search for the Cities of Gold. Ah-ah-ah-ah-ah... wishing for The Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold. Do-do-do-do ah-ah-ah, do-do-do-do, Cities of Gold. Do-do-do-do, Cities of Gold. Ah-ah-ah-ah-ah... some day we will find The Cities of Gold.\
\
There's a voice that keeps on calling me. Down the road, that's where I'll always be. Every stop I make, I make a new friend. Can't stay for long, just turn around and I'm gone again. Maybe tomorrow, I'll want to settle down, Until tomorrow, I'll just keep moving on.\
\
",
 ["url"] = "",
 ["id"] = 34,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 34

local rec = {
 ["post_id"] = 8,
 ["approved"] = true,
 ["body"] = "\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
I never spend much time in school but I taught ladies plenty. It's true I hire my body out for pay, hey hey. I've gotten burned over Cheryl Tiegs, blown up for Raquel Welch. But when I end up in the hay it's only hay, hey hey. I might jump an open drawbridge, or Tarzan from a vine. 'Cause I'm the unknown stuntman that makes Eastwood look so fine.\
\
",
 ["url"] = "",
 ["id"] = 35,
 ["author"] = "Larry",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 35

local rec = {
 ["post_id"] = 8,
 ["approved"] = true,
 ["body"] = "\
This is my boss, Jonathan Hart, a self-made millionaire, he's quite a guy. This is Mrs H., she's gorgeous, she's one lady who knows how to take care of herself. By the way, my name is Max. I take care of both of them, which ain't easy, 'cause when they met it was MURDER!\
\
Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. Birds taught me to sing, when they took me to their king, first I had to fly, in the sky so high so high, so high so high so high, so - if you want to sing this way, think of what you'd like to say, add a tune and you will see, just how easy it can be. Treacle pudding, fish and chips, fizzy drinks and liquorice, flowers, rivers, sand and sea, snowflakes and the stars are free. La la la la la, la la la la la la la, la la la la la la la, la la la la la la la la la la la la la, so - Barnaby The Bear's my name, never call me Jack or James, I will sing my way to fame, Barnaby the Bear's my name. \
\
",
 ["url"] = "",
 ["id"] = 36,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 36

local rec = {
 ["post_id"] = 5,
 ["approved"] = true,
 ["body"] = "\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
Thunder, thunder, thundercats, Ho! Thundercats are on the move, Thundercats are loose. Feel the magic, hear the roar, Thundercats are loose. Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thunder, thunder, thunder, Thundercats! Thundercats!\
\
",
 ["url"] = "",
 ["id"] = 37,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 37

local rec = {
 ["post_id"] = 5,
 ["approved"] = true,
 ["body"] = "\
Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. He's got style, a groovy style, and a car that just won't stop. When the going gets tough, he's really rough, with a Hong Kong Phooey chop (Hi-Ya!). Hong Kong Phooey, number one super guy. Hong Kong Phooey, quicker than the human eye. Hong Kong Phooey, he's fan-riffic!\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 38,
 ["author"] = "Moe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 38

local rec = {
 ["post_id"] = 4,
 ["approved"] = true,
 ["body"] = "\
Mutley, you snickering, floppy eared hound. When courage is needed, you're never around. Those medals you wear on your moth-eaten chest should be there for bungling at which you are best. So, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon, stop that pigeon. Howwww! Nab him, jab him, tab him, grab him, stop that pigeon now.\
\
",
 ["url"] = "http://www.keplerproject.org",
 ["id"] = 39,
 ["author"] = "Jane Doe",
 ["created_at"] = 1183757201,
 ["email"] = ""}
rec = t:new(rec)
rec:save(true)

-- Record 39

local rec = {
 ["post_id"] = 54,
 ["id"] = 40,
 ["approved"] = true,
 ["created_at"] = 1183757201,
 ["body"] = "\
<p>80 days around the world, we'll find a pot of gold just sitting where the rainbow's ending. Time - we'll fight against the time, and we'll fly on the white wings of the wind. 80 days around the world, no we won't say a word before the ship is really back. Round, round, all around the world. Round, all around the world. Round, all around the world. Round, all around the world.</p>\
"}
rec = t:new(rec)
rec:save(true)

-- Record 40

local rec = {
 ["post_id"] = 54,
 ["approved"] = true,
 ["body"] = "\
<p>Blatz blum <em>plaz</em>.</p>\
",
 ["id"] = 42,
 ["author"] = "Fabio Mascarenhas",
 ["created_at"] = 1183760795,
 ["email"] = "mascarenhas@gmail.com"}
rec = t:new(rec)
rec:save(true)

-- Record 41

local rec = {
 ["post_id"] = 6,
 ["id"] = 43,
 ["approved"] = true,
 ["created_at"] = 1183761355,
 ["body"] = "\
<p>Foo bar blaz.</p>\
"}
rec = t:new(rec)
rec:save(true)

-- Table user

local t = mapper:new('user')

-- Record 1

local rec = {
 ["id"] = 1,
 ["name"] = "Fabio Mascarenhas",
 ["login"] = "mascarenhas@acm.org",
 ["password"] = "password"}
rec = t:new(rec)
rec:save(true)

-- Table section

local t = mapper:new('section')

-- Record 1

local rec = {
 ["id"] = 1,
 ["description"] = "",
 ["title"] = "Blog Posts",
 ["tag"] = "blog-main"}
rec = t:new(rec)
rec:save(true)

-- Record 2

local rec = {
 ["id"] = 2,
 ["description"] = "",
 ["title"] = "Pages",
 ["tag"] = "pages"}
rec = t:new(rec)
rec:save(true)

-- Record 3

local rec = {
 ["id"] = 3,
 ["description"] = "",
 ["title"] = "Blogroll",
 ["tag"] = "blogroll"}
rec = t:new(rec)
rec:save(true)
