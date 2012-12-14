core.module_load ("./modules/seen.so")
core.module_load ("./modules/linktitle.so")

lostsig = client.new ("irc.lostsig.net")
lostsig.owner = "tm512@underhalls.net"

function lostsig:onConnect ()
	self:join ("#bottest")
end

lostsig:connect ()
