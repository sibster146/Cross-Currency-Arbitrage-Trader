from dotenv import load_dotenv
import os
from coinbase import jwt_generator
import sys

# Load environment variables from .env file
load_dotenv()
api_key = os.getenv("COINBASE_API_NAME")
secret_key = os.getenv("COINBASE_PRIVATE_KEY")

if secret_key:
    secret_key = secret_key.replace("\\n", "\n")
    
request_path = "/api/v3/brokerage/"

def main():

	args = sys.argv[1:]
	request_method = args[0]
	api_command = args[1]

	jwt_uri = jwt_generator.format_jwt_uri(request_method, request_path + api_command)
	jwt_token = jwt_generator.build_rest_jwt(jwt_uri, api_key, secret_key)
	print(jwt_token)

if __name__ == "__main__":
    main()
