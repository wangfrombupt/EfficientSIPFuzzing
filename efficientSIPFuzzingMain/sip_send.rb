## => sip_send.rb
## => 

class SipSend
	def initialize
		@server_uri = "127.0.0.1"
		@call_id = rand_str(28)
		@ip = "127.0.0.1"
		@cseq = "1"
		@from_user = "alice"
		@port = 5060
		@branch = "z9hG4bK-"
      	@branch << rand_str(27)
		@tag = "1000"
	end
	def rand_str( len )
		tokens = ("a".."z").to_a + ("A".."Z").to_a + ("0".."9").to_a  
		rand_str = ""  
		1.upto(len) { |i| rand_str << tokens[rand(tokens.size-1)] }  
		return rand_str
	end
end