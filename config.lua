lostsig = client.new ("irc.lostsig.net")
lostsig.owner = "tm512@underhalls.net"

function lostsig:onConnect ()
	self:join ("#bottest")
end

lostsig:connect ()
