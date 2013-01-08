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

channel = { }
channel.__index = channel

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

	self.channels = { }

	return self
end

function channel.new (cl, name)
	local self = { }
	setmetatable (self, channel)

	self.cl = cl
	self.name = name

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
	core.client_join (self.id, name, pass)
	self.channels [name] = channel.new (self, name)
	return self.channels [name] -- return the newly added channel
end

function client:part (name)
	core.client_part (self.id, name)
	self.channels [name] = nil
end

function client:raw (msg)
	core.client_raw (self.id, msg)
end

function client:privmsg (targ, msg)
	core.client_privmsg (self.id, targ, msg)
end

function channel:part ()
	self.cl:part (self.name)
end

function channel:privmsg (msg)
	self.cl:privmsg (self.name, msg)
end

module = { }
module.__index = module

function module.load (path)
	local self = { }

	self.name = core.module_load (path)
	return self
end

function module:unload ()
	core.module_unload (self.name)
	self.name = nil
end
