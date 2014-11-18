require 'socket'
require 'timeout'
require './file_management'
require './config'
require 'yaml'
class ClientSipTest
	def initialize(uip, uport, rip, rport, server_uri, from_user, to_user)
		@uip = uip
		@uport = uport
		@rip = rip
		@rport = rport
		@server_uri = server_uri
		@from_user = from_user
		@to_user = to_user
		@test_time = 2
		@call_id = ''
		@branch = ''
		@tag = "1000"
		@cseq = "1"
	end
	def test_preformat_sip_once(_sip='')
		@call_id = rand_str(28)
		@branch = "z9hG4bK-"
		@branch << rand_str(27)
		sip = _sip
		sip = deal_preformat_1(sip, '[Server_Uri]', @server_uri)
		sip = deal_preformat_1(sip, '[Call_Id]', @call_id)
		sip = deal_preformat_1(sip, '[IP]', @uip)
		sip = deal_preformat_1(sip, '[CSeq]', @cseq)
		sip = deal_preformat_1(sip, '[PORT]', @uport)
		sip = deal_preformat_1(sip, '[Tag]', @tag)
		sip = deal_preformat_1(sip, '[Branch]', @branch)
		sip = deal_preformat_1(sip, '[From_User]', @from_user)
		#puts sip
		sip
	end
	# replace _str with _replace in _template
	# usage:
	# deal_preformat_1(sip, '[Server_Uri]', @server_uri)
	# editor:jinguodong
	def deal_preformat_1(_template='', _str='', _replace='')
		template = _template
		while template[_str] != nil
			template[_str]= _replace
		end
		template
	end
	def test_format_sip_once(_sip='')
		if _sip == ''
			raise ArgumentError, "#{__FILE__}:#{__LINE__}: param error"
		end
		puts _sip
		r = '';	rr = ''
		begin
			res_thr2 = Thread.new do
				begin
					#puts req
					ss = UDPSocket.new
					ss.bind(@uip, 5070)
					timeout(@test_time+0.5){
						rr = ss.recv(4096)
						rr 
					}
				rescue Timeout::Error
					'timeout thr2'
				rescue Exception
					'error'
				ensure
					ss.close
				end
			end
		end
		sleep(0.1)
		begin
			res_thr1 = Thread.new do
				begin
					s = UDPSocket.new
					s.bind(@uip,@uport)
					s.connect(@rip,"5060")
					s.send(_sip, 0)
					timeout(@test_time){
						r = s.recv(4096)
						r
					}
				rescue Timeout::Error
					'timeout thr1'
				rescue Exception => err
					p err.message
					'error'
				ensure
					s.close
				end
			end
		end
		
		# p res_thr1.value[8, 3]+':'+res_thr2.value
		# puts res_thr1.value
		# puts res_thr2.value
		res_thr2.value

	end
	def test_format_sip_once_v1(_sip='')
		begin
			s = UDPSocket.new
			s.bind(@uip,@uport)
			s.connect(@rip,"5060")
			s.send(_sip, 0)
		rescue Timeout::Error
			'timeout thr1'
		rescue Exception => err
			p err.message
			'error'
		ensure
			s.close
		end
	end
	def rand_str( len )  
	  tokens = ("a".."z").to_a + ("A".."Z").to_a + ("0".."9").to_a  
	  rand_str = ""  
	  1.upto(len) { |i| rand_str << tokens[rand(tokens.size-1)] }  
	  return rand_str  
	end

	def deal_test_result(input={})
		tmp_name = '/tmp/mysips.log'
		is_in_deal = 0
		result_cnt = 0
		open(tmp_name).each { |line|
			case is_in_deal
			when 0
				if line =~ /[\d]*:sips/
					is_in_deal = 1
				end
			when 1
				if line =~ /method:REGISTER/
					is_in_deal = 2
				else
					is_in_deal = 0
				end
			when 2
				if line =~ /^end/
					is_in_deal = 3
				end
				if line =~ /test_account:(.*?):(.*?):(.*)/
					puts ""
					puts "#{$1}+#{$2}+#{$3}+#{$4}"
					file_name = $1
					func_name = $2
					vul_name = $3
					others = $4
					input['sip_info'].each do |sip_one_line|
						one_res_cnt = 0
						if sip_one_line['tested'] != 'true'
							sip_one_line['aims'].each do |aim|
								# puts aim['file']
								# puts aim['func']
								aim['file'] =~ /.*\/(.*)[.]c$/
								tem_file_name = $1
								tem_func_name = aim['func']
								puts "tem_name=#{tem_file_name}:tem_func=#{tem_func_name}"
								if aim['tested']!='true' && tem_file_name==file_name && tem_func_name==func_name 
									one_res_cnt = one_res_cnt+1
									result_cnt = result_cnt+1
									# mark the aim
									aim['tested']='true'
								end
							end
							# mark the sip one line, but the sip_one_line may be aimed to multi path
							# sip_one_line['tested'] = 'true'
							puts "one_res_cnt:#{one_res_cnt}"
							if sip_one_line['fact_value']<0
								sip_one_line['fact_value']=0
							end
							sip_one_line['fact_value'] = sip_one_line['fact_value'] + one_res_cnt
						end
					end
				end
			else
				line
			end
		} # end of the open file
		# mark the sip packet
		input['tested'] = 'true'
		input['fact_value'] = result_cnt
		result_cnt
	end
end

# uip = "10.109.247.207"
# uport = "5067"
# server_Uri = "10.109.247.207"
# rip = "10.109.247.207"
# rport = "5060"
# from_user = 'alice'
# to_user = 'alice'

# test =  ClientSipTest.new(uip, uport, rip, rport, server_Uri, from_user, to_user)

#test.deal_test_result()


# call_id = rand_str(28)
# branch = "z9hG4bK-"
# branch << rand_str(27)
# # req =  "REGISTER sip:#{server_Uri} SIP/2.0" + "\r\n"
# # req << "Call-ID: #{call_id}@#{uip}" + "\r\n"
# # req << "CSeq: 1 REGISTER"+"\r\n"

# # req << "Via: SIP/2.0/UDP #{uip}:#{uport};branch=#{branch}" + "\r\n"
# # req << "From: \"linfeng\" <sip:alice@#{server_Uri}>;tag=1000"+"\r\n"
# # req << "To: \"linfeng\" <sip:alice@open-ims.test>"+"\r\n"
# # #req << "Via: SIP/2.0/UDP #{uip}:#{port};branch=#{branch}" + "\r\n"
# # #req << "Route: <10.109.247.185:4060;lr>"+"\r\n"
# # req << "Max-Forwards: 20" +  "\r\n"
# # req << "Expires: 10800" +  "\r\n"
# # #req << "Authorization: Digest username=\"alice@#{server_Uri}\",realm=\"#{server_Uri}\",nonce=\"\",response=\"\",uri=\"sip:#{server_Uri}\"" +  "\r\n"
# # req << "Authorization: Digest username=\"alice@open-ims.test\",realm=\"#{server_Uri}\",nonce=\"\",response=\"\",uri=\"sip:#{server_Uri}\"" +  "\r\n"
# # req << "Supported: path" + "\r\n"
# # req << "Contact: <sip:#{uip}:#{uport}>" + "\r\n"
# # req << "P-Preferred-Identity: \"linfeng\" <sip:alice@#{server_Uri}>" + "\r\n"
# # req << "P-Access-Network-Info: 3GPP-UTRAN-TDD; utran-cell-id-3gpp=00000000" + "\r\n"
# # req << "Privacy: none" +  "\r\n"
# # req << "User-Agent: Fraunhofer FOKUS/NGNI Java IMS UserEndpoint FoJIE 0.1 (jdk1.3)" + "\r\n"
# # req << "Allow: INVITE,ACK,CANCEL,BYE,MESSAGE,NOTIFY" +  "\r\n"
# # req << "Content-Length: 0" +  "\r\n\r\n"

# # test.test_format_sip_once(req)

# sips_info = open("./format_sips/"+$YAML_FILE, 'r') { |f| YAML.load(f) }
# p sips_info
# store_path = './genetic/first/contact/malloc/'

# include FileManagement
# 25.times do |i|
# 	# sleep(3)
# 	begin
# 		tmp_sip = File.open("./format_sips/#{i+1}", 'r') do |file|
# 			file.read
# 		end
# 		format_sip = test.test_preformat_sip_once(tmp_sip)
# 		result_flag = test.test_format_sip_once(format_sip)
# 		puts result_flag
# 		# if match 'timeout' raise Exception to break the program
# 		if result_flag.match(Regexp.new('timeout'))
# 			sips_info[i] = nil
# 			raise Exception, "timeout: #{i}"
# 		end
# 		# write_to_file_force("#{store_path}#{i+1}.sip", tmp_sip)

# 	rescue Exception => err
# 		puts err.message
# 	end
# 	puts i
# 	STDOUT.flush
# end
# p sips_info
# open(store_path+$YAML_FILE, 'w') { |f| YAML.dump(sips_info, f) }

# # p open(store_path+$YAML_FILE, 'r') { |f| YAML.load(f) }