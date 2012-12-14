--[[
   ispolin
   Copyright [c] 2011-2012 tm512 (Kyle Davis), All Rights Reserved.

   Ispolin is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 3, as
   published by the Free Software Foundation.

   Ispolin is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Ispolin.  If not, see <http://www.gnu.org/licenses/>.
]]--

-- wrap the C functions up into a less ugly API

client = { }
client.__index = client

function client.new (host, port)
	local self = { }
	setmetatable (self, client)

	self.id = core.client_new (host, port or "6667")
	core.client_ref (self.id, self)

	self.host = host
	self.port = port

	self.nick = "ispolin"
	self.username = "ispolin"
	self.realname = "ispolin"

	self.prefix = "."
	self.owner = ""

	return self
end

function client:connect ()
	core.client_setcfg (self.id, self.nick, self.username, self.realname, self.prefix, self.owner)

	if not core.client_connect (self.id)
	then
		self.id = core.maxclients
	end
end

function client:join (name, pass)
	-- todo: create a "channel" type table
	core.client_join (self.id, name, pass)
end
