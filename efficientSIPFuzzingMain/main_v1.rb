## => file_name: main_v1.rb
## => describe: the main file that contral the flow of the application
## => author: jinguodong
## => time: 2014-1-3
## => some changes from main.rb
	# rebuild the main control flow
## => usage:
	# main = MainScan.new(sip_stack_version, test_path, test_sip_path)
	# main.file_analyze # => model the project of the test_sip_path
	# main.source_sip_stack(file_name, function_name) # => source the project by file_name and function_name.
	# => main.source_sip_stack return the headers of sip.
	# main.sip_generate(_sip_header="", _sip_type="")
	# => _sip_header: is the SIP header of the malformed sip packet that is going to be generated
	# => _sip_type: is the type of the SIP header of the malformed sip packet that is going to be generated
## => example:
	# sip_stack_version = "opensips_1_10"
	# test_path = `pwd`.chop! + "/#{sip_stack_version}"
	# test_sip_path = test_path+"/parser"
	# main = MainScan.new(sip_stack_version, test_path, test_sip_path)
	# file_name = test_sip_path+"/parse_to.c"
	# function_name = "parse_to_param"
	# p main.source_sip_stack(file_name, function_name)
## => edit 
	# add vulnerability_function.rb to deal with the vulnerability function
	# To get the format value of the vulnerability function:
	# VulnerabilityFunction.new().get_value_by_funcname('memcpy')
	# To get the file name and function name of the vulnerability function:
	# VulnerabilityFunction.new().get_info_of_funcname('memcpy')
	
require "./source_sip_stack"
require "./file_analyze"
require "./sip_mysql_cfg"
require "/home/jinguodong/workspace/sip_generate/malfSipMsgGen.rb"
require "./vulnerability_function.rb"
require "./format_sip_generate.rb"
require "yaml"
require "./client_sip_attack_value_test.rb"
require "./mydebug.rb"
require "./genetic_func.rb"

include MyDebug
include GenFunc

class MainScan
	def initialize(_sip_stack_version="opensips_1_10", _test_path="", _test_sip_path="")
		@sip_stack_version = _sip_stack_version
		@test_path =_test_path
		@test_sip_path = _test_sip_path
		@source_sip_stack = SourceSipStack.new(@sip_stack_version)
		@vulnerability = VulnerabilityFunction.new()

		# set ClientSipTest init param
		uip = "10.109.247.246"
		uport = "5067"
		server_Uri = "10.109.247.246"
		rip = "10.109.247.246"
		rport = "5060"
		from_user = 'alice'
		to_user = 'alice'
		@test_client =  ClientSipTest.new(uip, uport, rip, rport, server_Uri, from_user, to_user)
	end

	# To get the format value of the vulnerability function:
	def get_value_by_funcname(_func_name)
		@vulnerability.get_value_by_funcname(_func_name)
	end
	# To get the file name and function name of the vulnerability function:
	def get_info_of_funcname(_func_name)
		@vulnerability.get_info_of_funcname(_func_name)
	end

	def source_sip_stack(_file="", _function="", _other_info="")
		# test = SourceSipStack.new(sip_stack_version, "#{test_sip_path}/parse_to.c", "test", "strcpy")
		
		@source_sip_stack.set_other_info(_other_info)
		@source_sip_stack.source_sip_file("#{_file}", "#{_function}")#{test_sip_path}/parse_to.c
		@source_sip_stack.get_sip_header_from_source_result
	end
	def source_sip_stack_clear
		@source_sip_stack.clear
	end
	def source_result
		@source_sip_stack.get_source_result
	end

	def file_analyze
		# test on 2013_10_16_14_40 by jinguodong
		test = FileAnalyze.new(@sip_stack_version)
		stack_files = StaticFunction.get_files(@test_sip_path)
		stack_files.each do |f|
			test.set_file(f)
			test.file_cflow_v1
		end
	end
	def sip_generate(_sip_header="", _sip_type="")
		#define a MalfSipMsgGen object
		if _sip_header=="" && _sip_type==""
			# puts "System may generate all Malformed SIP packet, Yes or No?"
			# _user_input = gets()
			# if _user_input =~ /no/i
			# 	return
			# end
			return
		end
		sMsgGen=MalfSipMsgGen.new("REGISTER5")
		orgMsg=sMsgGen.getSipMsgFromFile("/home/jinguodong/workspace/sip_generate/template/REGISTER.txt")
		if _sip_header!="" && _sip_type==""
			sMsgGen.generateAllMalformedMsgbySipMsgElement(orgMsg,_sip_header)
		elsif _sip_header!="" && _sip_type!=""
			raise "#{__FILE__}:#{__LINE__}: param error"
			#sMsgGen.generateMalformedMsg(orgMsg,_sip_header,_sip_type)
		end
		#sMsgGen.generateAllMalformedMsg(orgMsg)
		# sMsgGen.generateAllMalformedMsgbySipMsgElement(orgMsg,"SIP-Content-Length")
	end
	def get_sip_template
		sMsgGen=MalfSipMsgGen.new("REGISTER5")
		orgMsg=sMsgGen.getSipMsgFromFile("/home/jinguodong/workspace/sip_generate/template/REGISTER.txt")
	end
	# sent the sip to tested nest
	def get_sip_from_pre_v1(pre_format_sip='')
		format_sip = @test_client.test_preformat_sip_once(pre_format_sip)
		# get final perform of the format sip.
		format_sip
	end
	# test format sip for once.
	def test_format_sip_v1(format_sip='')
		@test_client.test_format_sip_once_v1(format_sip)
	end
	# start sip server
	def start_sip_server
		`/home/jinguodong/open_project/opensips_1_10/opensips >/tmp/mysips.log 2>/dev/null &`
	end
	# kill sip server
	def kill_sip_server
		`kill -9 $(ps -ef | grep opensips | awk '{print $2}' )`
	end
	# start a test, set up the test env
	# after set up the test and end a test, call kill_sip_server will be suggested.
	# 
	def set_up_test_env
		`ruby expect_test_v1.rb`
	end
	def deal_result(input)
		@test_client.deal_test_result(input)
	end
end # end of MainScan

sip_stack_version = "opensips_1_10"
test_path = `pwd`.chop! + "/#{sip_stack_version}"
test_sip_path = test_path+"/parser"
# test_sip_path = test_path
main = MainScan.new(sip_stack_version, test_path, test_sip_path)

# analyze the test_sip_path, model the sip stack;
# main.file_analyze	
# puts "file_analyze complete"

# get the file and func of vulnerability function setted by other_info
other_info = "malloc"
file_func_array = main.get_info_of_funcname(other_info)
puts file_func_array


# middle_result store the middle result of control flow
# performance:
# [{'sip_header'=>, 'v_func'=>, 'file_name'=>, 'func_name'=>}, ...]
middle_result = []
file_func_array.each do |file_func|
	file_name = file_func["file_name"]
	function_name = file_func["function_name"]
	main.source_sip_stack_clear
	# if file_name =~ /parse_to/
	sip_tmp_result = main.source_sip_stack(file_name, function_name, other_info)
	# puts file_func["file_name"]
	# puts file_func["function_name"]
	# p sip_tmp_result
	# p sip_tmp_result.empty?
	if !sip_tmp_result.empty?
		sip_tmp_result.each do |key, value|
			# p key
			middle_result << {'sip_header'=>key, 'v_func'=>other_info, 'file_name'=>file_func['file_name'], 'func_name'=>file_func['function_name']}
		end
	end
end
middle_result.uniq!
# puts middle_result
# puts middle_result.size

# deal with middle_result, get the preform of first crow, as follows:
# [{:sip_info=>[{:sip_header, :value, :v_func, :aims=>[{:file, :func},..], :expect_value, :fact_value}, ...], :expect_value, :fact_value}, ...]
# 0...i 	[0, i-1]
# first_crow_pre the preform of first crow of genetic.
first_crow_pre=[]
for ele1 in (0...middle_result.size)
	# puts middle_result[ele1]
	tem_result = {}
	if middle_result[ele1]['vis'] != 'true'
		middle_result[ele1]['vis'] = 'true'
		tem_result = { 
			'sip_info'=>[
				{
					'sip_header'=>middle_result[ele1]['sip_header'],
					'value'=>'',
					'v_func'=>middle_result[ele1]['v_func'],
					'aims'=>[
						{
							'file'=>middle_result[ele1]['file_name'],
							'func'=>middle_result[ele1]['func_name']
						}
					],
					'expect_value'=>1,
					'fact_value'=>-1,
				}
			],
			'expect_value'=>1,
			'fact_value'=>-1
		}
		for ele2 in (0...middle_result.size)
			if middle_result[ele2]['vis']!='true' && 
				middle_result[ele2]['sip_header']==middle_result[ele1]['sip_header'] && 
				middle_result[ele2]['v_func']==middle_result[ele1]['v_func'] &&	
				middle_result[ele2]['value']==middle_result[ele1]['value']

				middle_result[ele2]['vis']='true'
				tem_result['sip_info'][0]['aims']<<{'file'=>middle_result[ele2]['file_name'], 'func'=>middle_result[ele2]['func_name']}
				tem_result['sip_info'][0]['expect_value']=tem_result['sip_info'][0]['expect_value']+1
				tem_result['expect_value']=tem_result['expect_value']+1
			end
		end
		# puts tem_result
		first_crow_pre<<tem_result
	end
end
# puts 'first_crow_pre:'
# puts first_crow_pre


# get related SIP header param array header
# $SIP_MYSQL(init in sip_mysql_cfg.rb) store the relation of sip_header and sip params.
# get first_crow_pre1 with first_crow_pre and $SIP_MYSQL, perform as follows:
# [{:sip_info=>[{:sip_param, :value, :v_func, :aims=>[{:file, :func},..], :expect_value, :fact_value}, ...], :expect_value, :fact_value}, ...]
first_crow_pre1 = []
first_crow_pre.each do |ele|
	tem = {}
	tem = ele.dup
	k = tem['sip_info'][0]['sip_header']
	$SIP_MYSQL[k].each do |_header|
		tem['sip_info'][0]['sip_header'] = _header
		tem['sip_info'][0]['sip_header_v1'] = k
		# puts tem
		tem1 = {}
		tem1 = tem.dup
		eval "first_crow_pre1 << #{tem1.to_s}"
		# main.sip_generate(_header)
	end
end
# puts "first_crow_pre1:"
# puts first_crow_pre1

# get the format value of the vulnerability function:
format_value_array = main.get_value_by_funcname(other_info)
# puts "format_value_array"
# puts format_value_array

# first crow
# [{:sip_header=>"", :value=>"", :value_class=>30}, ..]
# Change to 
# [{:sip_info=>[{:sip_header, :value, :value_id, :value_class, :v_func, :aims=>[{:file, :func},..], :expect_value, :infect_value}, ...], 
# :expect_value, :infect_value}, ...]
# edit at 2014-1-3 by jinguodong
# get first_crow finaly.
first_crow = []
first_crow_pre1.each do |ele1|
	tem = ele1
	format_value_array = main.get_value_by_funcname(tem['sip_info'][0]['v_func'])
	# puts format_value_array
	format_value_array.each do |value|
		tem['sip_info'][0]['value'] = value['value']
		tem['sip_info'][0]['value_id'] = value['value_id']
		tem['sip_info'][0]['value_class'] = value['value_class']
		# puts tem
		eval "first_crow << #{tem.to_s}"
	end
end
# puts "first_crow"
puts first_crow

# start the service of opensips
# main.set_up_test_env

# generate the first crow with first_crow array.
sip_res = FormatSipGenerate.new
# format_sip = ''
file_path = './format_sips/'
# `rm -f ./format_sips/*`
i=0
# first_crow.each do |input|
# 	format_sip = sip_res.get_premessage_from_template(input['sip_info'], main.get_sip_template)
	
# 	# start the service of opensips
# 	main.set_up_test_env
# 	sleep(1)
# 	# test format sip for once.
# 	format_sip_to_send = main.get_sip_from_pre_v1(format_sip)
# 	main.test_format_sip_v1(format_sip_to_send)

# 	# kill sip server, after the end of a test.
# 	sleep(1.5)
# 	main.kill_sip_server
# 	sleep(0.5)

# 	# here, deal with the result of this test.
# 	fact_value = main.deal_result(input)

# 	i=i+1
# 	# sip_res.write_to_file(format_sip, "#{file_path}#{i}")
# end

# store the test result of first crow to 'result_first_crow.yaml'
# open('result_first_crow.yaml', 'w') { |f| YAML.dump(first_crow, f) }

# read the test result of first crow from 'result_first_crow.yaml'
# result_first_crow = open('result_first_crow.yaml', 'r') { |f| YAML.load(f) }

# filter the crow with the attack_value=0 to first_crow with
# gened_first_crow perform the first crow which is filtered by expert test.
# puts ""
# puts "result_first_crow:"
# gened_first_crow = result_first_crow.delete_if do |input|
# 	input['fact_value'] == 0
# end

# store the gened first crow to 'gened_crow_1.yaml'
# open('gened_crow_1.yaml', 'w') { |f| YAML.dump(gened_first_crow, f) }

# read the test gened of first crow from 'gened_crow_1.yaml'
gened_crow_1 = open('gened_crow_1.yaml', 'r') { |f| YAML.load(f) }

t_cnt = 0
f_cnt = 0
v_cnt = 0
c_cnt = 0
gened_crow_1.each do |gen|
	case gen['sip_info'][0]['sip_header_v1']
	when 'To'
		t_cnt = t_cnt+1
	when 'From'
		f_cnt = f_cnt+1
	when 'Via'
		v_cnt = v_cnt+1
	when 'Contact'
		c_cnt = c_cnt+1
	end
end
puts "#{t_cnt}:#{f_cnt}:#{v_cnt}:#{c_cnt}"

# add %s(1024), %s(10240) to gened_crow_1
gened_crow_1_added = []
gened_crow_1.each do |input|
	eval "gened_crow_1_added << #{input.to_s}"
	if(input['sip_info'][0]['value'] == 'a(1024)')
		input['sip_info'][0]['value'] = "%s(1024)"
		eval "gened_crow_1_added << #{input.to_s}"
		input['sip_info'][0]['value'] = "a(1024)"
	end
	if(input['sip_info'][0]['value'] == 'a(10240)')
		input['sip_info'][0]['value'] = "%s(10240)"
		eval "gened_crow_1_added << #{input.to_s}"
		input['sip_info'][0]['value'] = "a(10240)"
	end
end
# puts "gened_crow_1_added"
# puts gened_crow_1_added

# store gened_crow_1 to dir and files.
def write_sips_to_file(sips, top_dir, second_dir, sip_res, main)
	if(Dir.exists?("#{top_dir}/#{second_dir}"))
	else
		if(!Dir.exists?(top_dir))
			`mkdir #{top_dir}`
		end
		`mkdir #{top_dir}/#{second_dir}`
	end

	dir = "#{top_dir}/#{second_dir}/"

	i = 0
	sips.each do |input|
		format_sip = sip_res.get_premessage_from_template(input['sip_info'], main.get_sip_template)
		sip_res.write_to_file(format_sip, "#{dir}#{i}")
		i = i+1
	end
end
# write_sips_to_file(gened_crow_1_added, 'format_sips', 1, sip_res, main)


# get a clone of the first crow with no dulpulicate SIP-Header
no_dul_sips = []
gened_crow_1_added.each do |input|
	if(no_dul_sips.size() == 0)
		eval "no_dul_sips << #{input.to_s}"
	else
		if(no_dul_sips[no_dul_sips.size() - 1]['sip_info'][0]['sip_header'] != input['sip_info'][0]['sip_header'])
			eval "no_dul_sips << #{input.to_s}"
		end
	end
end
# puts "no_dul_sips"
# puts no_dul_sips

# deal with the report
def deal_report(reports, gened_crow_1_added)
	gen_values = []
	reports.each do |rid|
		# puts gened_crow_1_added[rid]
		tem_value = gened_crow_1_added[rid]['sip_info'][0]['value']
		tem_class = gened_crow_1_added[rid]['sip_info'][0]['value_class']
		tem_values = gen_values_by_report_2(tem_value, tem_class)
		tem_values.each do |inp_v|
			gen_values << inp_v
		end
	end
	gen_values
end
puts "reports"
format_reports = [0, 35]
gen_values = deal_report(format_reports, gened_crow_1_added)
puts gen_values

#
def get_format_values_fixed_length()
	res = gen_values_by_report_2_fixed_length("%s(1024)", "10", 1024, 1)
	res
end


# next genetic crow
def get_next_genetic(no_dul_sips, gen_values)
	next_genetic = []
	no_dul_sips.each do|sip|
		gen_values.each do|v|
			tem_obj = Object.new
			eval "tem_obj = #{v}"
			# puts tem_obj
			sip['sip_info'][0]['value_class'] = tem_obj['class']
			sip['sip_info'][0]['value'] = tem_obj['value']
			eval "next_genetic << #{sip.to_s}"
		end
	end
	next_genetic
end
next_genetic = get_next_genetic(no_dul_sips, gen_values)
# puts "next_genetic"
# puts next_genetic

# add for experiments of 'memcpy'
# "Via" => ["SIP-Via", "SIP-Via-Host", "SIP-Via-Hostcolon", "SIP-Via-Hostport", "SIP-Via-Version", "SIP-Via-Tag"],
# "From" => ["SIP-From", "SIP-From-Displayname", "SIP-From-Tag", "SIP-From-Colon", "SIP-From-URI-SIP", "SIP-From-URI-User", "SIP-From-URI-Host"],
# "Contact" => ["SIP-Contact", "SIP-Contact-Displayname", "SIP-Contact-URI", "SIP-Contact-Left-Paranthesis", "SIP-Contact-Right-Paranthesis"],
# "To" => ["SIP-To", "SIP-To-Displayname", "SIP-To-URI", "SIP-To-Tag", "SIP-To-Left-Paranthesis", "SIP-To-Right-Paranthesis"],
# "Expires" => ["SIP-Expires"],

test_func = "memcpy"
memcpy_sips = []
values = ["%*d(10000)", "%n(15024)", "%p(15024)", "%s(15024)", "%*s(10024)", "%d(15024)", "%z2000000d(3024)", "%z0200000d(3024)", "%.2000000f(3024)"]
values = ["a(31000)", "%*d(10000)", "%n(15024)", "%p(15024)", "%s(15024)", "%*s(10024)"]
# memcpy_sips << {"sip_info"=>[{"sip_header"=>"SIP-To-Right-Paranthesis", "value"=>"%s(15240)", "value_class"=>"30"}]}

sip_headers = ["Contact", "Expires"]
sip_headers = ["Contact"]
sip_headers = ["To"]
sip_headers.each do |_sip|
	$SIP_MYSQL[_sip].each do |_header|
		values.each do |_value|
			memcpy_sips << {"sip_info"=>[{"sip_header"=>"#{_header}", "value"=>"#{_value}", "value_class"=>"30"}]}		
		end
	end
end

write_sips_to_file(memcpy_sips, "format_sips", test_func, sip_res, main)
open("./format_sips/#{test_func}/sips.yaml", 'w') { |f| YAML.dump(next_genetic, f) }
puts "OK!!!!!"

# write_sips_to_file(next_genetic, "format_sips", "2_2", sip_res, main)
# open("./format_sips/2_2/sips.yaml", 'w') { |f| YAML.dump(next_genetic, f) }
# puts "OK!!!!!"


# format_reports = [104, 87]
# next_genetic[104]['sip_info'][0]['value'] = "%s(32573)"
# next_genetic[87]['sip_info'][0]['value'] = "%V(29400)"

# gen_values = deal_report(format_reports, next_genetic)
# puts gen_values
# next_genetic = get_next_genetic(no_dul_sips, gen_values)

# write_sips_to_file(next_genetic, "format_sips", "3", sip_res, main)
# open("./format_sips/3/sips.yaml", 'w') { |f| YAML.dump(next_genetic, f) }




# puts next_genetic
# write_sips_to_file(gened_crow_1_added, file_path, sip_res, main)
# => write gened_crow_1_added to file_path/i.
# deal_report(reports, gened_crow_1_added)
# => return next_values
# get_next_genetic(no_dul_sips, gen_values)
# => get next_genetic_array


# main.do_genetic(gened_crow_1, n)
# gened_crow_1 means the first_crow of the genetic algorithm.
# n mians the deeping of genetic.
# return:
# 	the final of the genetic.
# 	



# kill sip server, after the end of a test.
# sleep(0.1)
# main.kill_sip_server

# # store the info of the sip packet.
# open(file_path+$YAML_FILE, 'w') { |f| YAML.dump(first_crow, f) }
# puts "Total:#{first_crow.size}:#{gened_first_crow.size}"


