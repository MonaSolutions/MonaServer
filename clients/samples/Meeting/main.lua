
meeters = {}

function onConnection(client, userName, meeting)
	
	if client.protocol == "RTMFP" or client.protocol == "RTMP" then
		meeter = {}
		meeter.userName = userName
		meeter.meeting = meeting
		
		INFO("User connected: ", meeter.userName , " meeting: ", meeter.meeting)
		
		sendParticipantUpdate(meeter.meeting)
		meeters[client] = meeter -- Add participant to the list
	end
	
	function client:getParticipants(meeting)
		result = {}
		i = 0;
		for cur_client, cur_meeter in pairs(meeters) do
			if (cur_meeter.meeting == meeting) then
				i = i+1;
				if cur_client.id then
					cur_meeter.protocol = 'rtmfp'
				end
				cur_meeter.farID = cur_client.id;		
				result[i] = cur_meeter
			end
		end	
		return result
	end
		
	function client:sendMessage(meeting, from, message)
  
		for cur_client, cur_meeter in pairs(meeters) do
			if (cur_meeter.meeting == meeting) then
				cur_client.writer:writeInvocation("onMessage", from, message)
			end
		end
	end
	
	return {index="VideoMeeting.html"}
end

function onDisconnection(client)
  meeter = meeters[client]

  if meeter then
    INFO("User disconnecting: "..meeter.userName)
    meeters[client] = nil
    sendParticipantUpdate(meeter.meeting)
  end
end

function sendParticipantUpdate(meeting)
	for cur_client, cur_meeter in pairs(meeters) do
		if (cur_meeter.meeting == meeting) then
			cur_client.writer:writeInvocation("participantChanged")
		end
	end
end