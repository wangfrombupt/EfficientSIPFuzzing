## => file_name: main.rb
## => describe: the main file that contral the flow of the application
## => author: jinguodong
## => time: 2013-10-29
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
require "/opt/SipGenerator_ruby/malfSipMsgGen.rb"
require "./vulnerability_function.rb"
require "./format_sip_generate.rb"
require "yaml"

class MainScan
	def initialize(_sip_stack_version="opensips_1_10", _test_path="", _test_sip_path="")
		@sip_stack_version = _sip_stack_version
		@test_path =_test_path
		@test_sip_path = _test_sip_path
		@source_sip_stack = SourceSipStack.new(@sip_stack_version)
		@vulnerability = VulnerabilityFunction.new()
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
		orgMsg=sMsgGen.getSipMsgFromFile("/opt/SipGenerator_ruby/template/REGISTER.txt")
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
		orgMsg=sMsgGen.getSipMsgFromFile("/opt/SipGenerator_ruby/template/REGISTER.txt")
	end
end

sip_stack_version = "opensips_1_10"
test_path = `pwd`.chop! + "/#{sip_stack_version}"
test_sip_path = test_path+"/parser"
# test_sip_path = test_path
main = MainScan.new(sip_stack_version, test_path, test_sip_path)

# analyze the test_sip_path, model the sip stack;
# main.file_analyze	
# puts "file_analyze complete"

# source the sip stack, by file_name and function_name;
file_name = test_sip_path+"/contact/contact.c"
function_name = "parse_contacts"
other_info = "malloc"
# sip_result consist of the sip header related to the test file_name and function_name;
# 
sip_result = main.source_sip_stack(file_name, function_name, other_info)
puts "SIP Result:"
p sip_result
# main.source_result consist of the source route of the test file_name and function_name in the sip stack
puts "Source Result:"
puts main.source_result

# get related SIP header param array header
header_array = []
sip_result.each do |k, v|
	if v == "true"
		$SIP_MYSQL[k].each do |_header|
			header_array << _header
			# main.sip_generate(_header)
		end
	end
end
#puts header_array

# get the format value of the vulnerability function:
format_value_array = main.get_value_by_funcname(other_info)
format_value_array[format_value_array.size] = {"value"=>"%z020000d(1024)", "value_class"=>20}
# puts format_value_array
puts main.get_info_of_funcname(other_info)

# [{:sip_header=>"", :value=>"", :value_class=>30}, ..]
# Change to 
# [{:sip_info=>[{:sip_header, :value, :v_func, :aims=>[{:file, :func},..], :expect_value, :infect_value}, ...], 
# :expect_value, :infect_value}, ...]
# edit at 2014-1-3 by jinguodong
# format_input
format_input = []
header_array.each do |header|
	format_value_array.each do |value|
		tem_arr = []
		tem_arr << {"sip_header"=>header, "value"=>value['value'], "value_class"=>value['value_class'], 'value_id'=>value['value_id'], 'v_func' => other_info, 'aims'=>[{'file_name'=>file_name, 'func_name'=>function_name}], 'expect_value'=>1, 'infect_value'=>0}
		format_input << {"sip_info"=>tem_arr, "expect_value"=>1, "infect_value"=>0}
	end
end
# test format_input
# format_input << [{"sip_header"=>"To", "value"=>"%z020000d(1024)", "value_class"=>20}]
puts format_input

sip_res = FormatSipGenerate.new
format_sip = ''
file_path = './format_sips/'
`rm -f ./format_sips/*`
i=0
# generate the first crow with format_input array.
# format_input.each do |input|
# 	format_sip = sip_res.get_premessage_from_template(input, main.get_sip_template)
# 	i=i+1
# 	sip_res.write_to_file(format_sip, "#{file_path}#{i}")
# end
open(file_path+$YAML_FILE, 'w') { |f| YAML.dump(format_input, f) }

puts "Total:#{i}:#{format_input.size}"


