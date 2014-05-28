
FAQ
##############################

.. contents:: Table of Contents

What are the advantages of RTMFP?
*******************************************

**RTMFP (Real Time Media Flow Protocol)** is the most powerful protocol to deliver live real-time communication for these principal reasons:

- RTMFP supports Native IP Multicast, Application Level Multicasting (using P2P), and the combination of both called Fusion Multicasting.
- RTMFP is based on the User Datagram Protocol (UDP), which increases the speed of delivery; Audio and Video packets are not forced to be retransmitted, which greatly reduces latency.
- RTMFP utilizes peer-to-peer (P2P) assisted network communication (end-users communicate directly), which consumes minimal server bandwidth and system resources.

Why doesn't the `Cirrus Sample Application`_ work?
***************************************************

Cirrus Server requires the following script:

.. code-block:: lua

	function onConnection(client,...)
		function client:relay(targetId,...)
			target = mona.clients(targetId)
			if not target then
				error("client '"..targetId.."' not found")
				return
			end
			target.writer:writeInvocation("onRelay",self.id,...)
			target.writer:flush(true)
		end
	end

You can add the script (seen above) in *www/main.lua* to make the `Cirrus Sample Application`_ work. 

.. Note:: The `Cirrus Sample Application`_ requires some python scripts to exchange user names, which can be achieved much easier with LUA code. Take a look at the *Meeting Sample* in `Samples <./samples.html>`_ page which does the same thing, but without other external dependencies.

.. _`Cirrus Sample Application`: http://labs.adobe.com/technologies/cirrus/samples/
