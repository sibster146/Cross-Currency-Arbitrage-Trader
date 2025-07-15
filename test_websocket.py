from coinbase.websocket import WSClient
import os
from dotenv import load_dotenv
import sys
import json

class Websocket:
    def __init__(self):
        self.sequence_num = 0
        load_dotenv()
        api_key = os.getenv("COINBASE_API_NAME")
        secret_key = os.getenv("COINBASE_PRIVATE_KEY")
        if secret_key:
            secret_key = secret_key.replace("\\n", "\n")

        self.ws_client = WSClient(api_key=api_key, api_secret=secret_key, on_message=self.on_message, verbose=False)

        self.pipe_path = "/tmp/datapipeline"
        if not os.path.exists(self.pipe_path):
            os.mkfifo(self.pipe_path)

        print("Waiting for C++ reader...", file=sys.stderr)
        self.pipe = open(self.pipe_path, "w", buffering=1)
        print("Pipe opened for writing.", file=sys.stderr)

    def on_message(self, msg):
        try:
            msg_json = json.loads(msg)
            update_seq_num = msg_json["sequence_num"]
            if self.sequence_num != update_seq_num:
                print("Sequence Number is Wrong", file=sys.stderr)
                return
            self.sequence_num += 1

            self.pipe.write(json.dumps(msg_json) + "\n")
            self.pipe.flush()
        except Exception as e:
            print(f"Exception in on_message: {e}", file=sys.stderr)
            self.shutdown()

    def shutdown(self):
        try:
            # self.pipe.write("hihi\n")
            self.pipe.flush()
            self.ws_client.close()
            self.pipe.close()
            print("Closing websocket and pipe.", file=sys.stderr)
        except Exception as e:
            print(f"Error during shutdown: {e}", file=sys.stderr)

    def run(self, symbols, duration):
        try:
            self.ws_client.open()
            self.ws_client.subscribe(symbols, ["ticker"])
            self.ws_client.sleep_with_exception_check(duration)
        finally:
            self.shutdown()

if __name__ == "__main__":
    print("Starting WebSocket...", file=sys.stderr)
    if len(sys.argv) > 2:
        str_product_ids = sys.argv[1]
        product_ids = str_product_ids.split(",")
        duration = int(sys.argv[2])
        socket = Websocket()
        socket.run(product_ids, duration)
    else:
        print("No argument received.", file=sys.stderr)
