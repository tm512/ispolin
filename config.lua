setuser ("ispolin", "ispolin", "ispolin")
setprefix (".")

crimson = irc_connect ("irc.lostsig.net", 6667)
irc_setowner (crimson, "tm512@underhalls.net")
irc_addchannel (crimson, "#bottest")
irc_addchannel (crimson, "#underhalls")
