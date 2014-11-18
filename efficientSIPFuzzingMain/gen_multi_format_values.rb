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

sip_res = FormatSipGenerate.new

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

def get_sip_template
	sMsgGen=MalfSipMsgGen.new("REGISTER5")
	orgMsg=sMsgGen.getSipMsgFromFile("/home/jinguodong/workspace/sip_generate/template/REGISTER.txt")
end

def write_sips_to_file(sips, top_dir, second_dir, sip_res)
	if(Dir.exists?("#{top_dir}/#{second_dir}"))
	else
		if(!Dir.exists?(top_dir))
			`mkdir #{top_dir}`
		end
		`mkdir -p #{top_dir}/#{second_dir}`
	end

	dir = "#{top_dir}/#{second_dir}/"

	i = 0
	sips.each do |input|
		format_sip = sip_res.get_premessage_from_template(input['sip_info'], get_sip_template)
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

#
def get_format_values_fixed_length(length=1024, types=1)
	res = gen_values_by_report_2_fixed_length("%s(1024)", "10", length, types)
	res
end

def get_format_values_fixed_type(base=1024, step=5000)
	res = gen_values_by_report_2_fixed_type("%s(1024)", "10", base, step)
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

gen_values = get_format_values_fixed_length(16024, 4)
# gen_values = get_format_values_fixed_type(51024, 5000)

next_genetic = get_next_genetic(no_dul_sips, gen_values)
# p next_genetic

write_sips_to_file(next_genetic, 'format_sips', 'types_16240/4', sip_res)
# write_sips_to_file(next_genetic, 'format_sips', 'length/51024', sip_res)
# write_sips_to_file(gened_crow_1_added, 'format_sips', 1, sip_res, main)



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
