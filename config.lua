setuser ("ispolin", "ispolin", "ispolin")
setprefix (".")

crimson = irc_connect ("crimson.lostsig.net", 6667)
irc_setowner (crimson, "tm512@crimson.lostsig.net")
irc_addchannel (crimson, "#bottest")
