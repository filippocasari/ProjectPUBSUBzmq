import zmq


def main():

    context = zmq.Context()

    # Socket facing producers
    frontend = context.socket(zmq.XPUB)
    frontend.bind("tcp://169.254.124.70:5602")

    # Socket facing consumers
    backend = context.socket(zmq.XSUB)
    backend.connect("tcp://169.254.157.194:5600")

    zmq.proxy(frontend, backend)

    # We never get here…
    frontend.close()
    backend.close()
    context.term()

if __name__ == "__main__":
    main()