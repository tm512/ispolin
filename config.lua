seen = module.load ("./modules/seen.so")
linktitle = module.load ("./modules/linktitle.so")

lostsig = client.new ("irc.lostsig.net")
lostsig.owner = "tm512@underhalls.net"

function lostsig:onConnect ()
	self:join ("#bottest")
end

lostsig:connect ()
