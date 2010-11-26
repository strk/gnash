from rtmpy import server

from twisted.internet import reactor


class LiveApplication(server.Application):
    """
    The simplest application possible.
    """

    def startup(self):
        print("Startup")

    def onAppStart(self):
        print("App start")
    
    def onAppStop(self):
        print("App stop")

    def onConnect(self, client, **args):
        print("Connection")
        client.call("initial", [ "connection attempt received", args ])
        print(args)
        return True

    def onConnectAccept(self, client, **args):
        print("Connection accepted", client)
        client.call("welcome", [ "You have connected!", args ])
        return True

    def onDownstreamBandwidth(self, interval, bandwidth):
        print("Downstream Bandwidth:", interval, bandwidth)
        return True

    def echo(self, *args, **kw):
        print("echo", args, kw)
        return args

    def onDisconnect(self, client):
	# This is probably never actually sent.
	client.call("disconnected", "arg1");
        return True;


app = LiveApplication()

reactor.listenTCP(9984, server.ServerFactory({
    'rtmpyecho': app
}))

reactor.run()
