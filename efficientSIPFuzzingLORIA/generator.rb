class MalfSipMsgGen
    def initialize(path="test", fName="./REGISTER.txt", msgelement=[], msgElCount=25)
        @path = path
        #畸形消息的存放目录（CANCEL，REGISTER……）
        @fpMsgName = ""
        #sip message model
        @orgMsg = getSipMsgFromFile(fName)
        #哪个字段的畸形
        @fpSipMsgElement = if msgelement==[] then getAllMsgEl(@orgMsg) else msgelement end
        #总共创建的畸形消息的数目
        @resCount = 0
        #某个字段上创建的畸形消息的数目
        @msgElCount = msgElCount
        #这是标准中的定义，换行以"\r\n”为标识
        @strKeyEnter = "\r\n"
    end

    #read模版消息字符串
    def getSipMsgFromFile(fName)
        fp=File.open(fName,"r")
        strMsg=""
        fp.each_line do |line|
            strMsg = strMsg+line
        end
        fp.close()
        #畸形消息的存放目录（CANCEL，REGISTER……）,delete the old files
        from_index = fName.index("/") + 1
        len = fName.index(".txt") - from_index
        @fpMsgName=fName[from_index, len]
        return strMsg
    end

    #the final function, we use this function directly
    def generateAllMalformedMsg()
        for msgelement in @fpSipMsgElement do
            puts "SipMsgElement---#{msgelement}"
            msg = deleteDes(@orgMsg, msgelement)
            generateAllMalformedMsgbySipMsgElement(msg, msgelement)
            puts "**********************creat #{@resCount} files!*********************"
        end
    end

    def generateAllMalformedMsgbySipMsgElement(orgMsg, msgEl)
        if orgMsg.index(msgEl)==nil
            return -1
        end
        for i in 1..@msgElCount do
            strRp = getRandValue(msgEl)
            buff=recreatMsgbySipMsgEl(orgMsg,msgEl,strRp)
            if buff==-1
                return -1
            else
                saveSipMsgToFile(buff, msgEl)
                @resCount+=1
            end
        end
        return 0
    end

    #根据指定字段msgEl，异常取值strRep,以及字段界定符signal_2对消息进行重组
    def recreatMsgbySipMsgEl(orgMsg,msgEl,strRep)
        #根据指定字段确定位置
        pos1=orgMsg.index('!'+msgEl+'!')
        pos2=pos1+msgEl.length+2
        finalMsg=""
        #don"t have the msgEl
        if pos1==nil
            return -1
        else
            finalMsg += orgMsg[0, pos1]
            finalMsg += strRep
            finalMsg += orgMsg[pos2, orgMsg.length-1-pos2]
        end
        return finalMsg
    end

    #将消息保存到文件
    def saveSipMsgToFile(strMsg, msgEl)
        #路径
        fpath=creatDir(msgEl)
        #文件名
        fileName=setFileName(fpath)

        fp=File.open(fileName,"w")
        fp.write(strMsg)
        fp.close
    end

    #不存在的话创建该目录
    def creatDir(msgEl)
        #畸形消息生成目录，是该程序的上层目录
        fPath = @path
        #创建目录的命令行
        cmdMkdir="mkdir -m 777 "
        cmdRm="rm -r "
        cmdMkdirWithPath=cmdMkdir+fPath
        if !(FileTest::exists?(fPath))
            `#{cmdMkdirWithPath}`
        end

        #创建指定名称目录的命令行
        cmdMkdirWithPath=cmdMkdir+fPath
        cmdRmPath=cmdRm+fPath
        if !(FileTest::exists?(fPath))
            `#{cmdMkdirWithPath}`
        end
        #是何种畸形元素
        fPath+="/"+msgEl
        cmdMkdirWithPath=cmdMkdir+fPath
        cmdRmPath=cmdRm+fPath
        if !(FileTest::exists?(fPath))
            `#{cmdMkdirWithPath}`
        end
        return fPath
    end


    #在某路径下创建文件，并返回文件名
    def setFileName(fpath)
        fileName=""
        pathDevide="/"
        fileName += fpath
        fileName += pathDevide
        fileName += @resCount.to_s
        return fileName
    end

    #delete the description in the message
    def deleteDes(orgMsg, msgEl)
        if msgEl=='SIP-Request-URI-CRLF' then
            msg = orgMsg[0, 109] + "!SIP-Request-URI-CRLF!" + orgMsg[134,1312]
        else
            reg = Regexp.new("\\{\\?#{msgEl}\\?.+?\\}")
            msg = orgMsg.sub(reg, "!#{msgEl}!")
        end
        length=msg.length
        finalMsg=""
        flag=true
        i=0
        while i<length
            tmp=msg[i,1]
            if tmp=="?"
                flag=!flag
            elsif flag && !(tmp=="{") && !(tmp=="}")
                finalMsg = finalMsg+tmp
            end
            i=i+1
        end
        return finalMsg
    end

    def getRandValue(msgEl)
        temp = ""

        maxlenglist = [ 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024,
                        5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
                        10240, 10240, 10240, 10240, 10240, 10240, 10240, 10240, 10240,
                        15360, 15360, 15360, 15360, 15360, 15360, 15360, 15360,
                        20480, 20480, 20480, 20480, 20480, 20480, 20480,
                        25600, 25600, 25600, 25600, 25600, 25600,
                        30720, 30720, 30720, 30720, 30720,
                        35840, 35840, 35840, 35840,
                        40960, 40960, 40960,
                        46080, 46080,
                        51200 ]

        chrlist = getRandChrList(msgEl)

        for i in 0..rand(maxlenglist[rand(66)]) do
            temp += chrlist[rand(chrlist.length)].chr()
        end
        return temp
    end

    def getRandChrList(msgEl)
        alphaup = (65..90).to_a
        alphalow = (97..122).to_a
        num = (48..57).to_a
        alphanum = alphaup + alphalow + num
        word = alphanum + [33,34,37,39,40,41,42,43,45,46,47,58,60,62,63,91,92,93,95,96,123,125,126]
        token = alphanum + [33, 37, 39, 42, 43, 45, 46, 95, 96, 126]

        case msgEl
            when 'SIP-Request-URI-CRLF' then
                return [9,10,13,32]
            when 'SIP-Call-Id-At' then
                return [64]
            when 'SIP-From-Colon','SIP-Via-Hostcolon' then
                return [58]
            when 'SIP-Cseq-Integer','SIP-Content-Length' then
                return num
            when 'SIP-To-Left-Paranthesis','SIP-Contact-Left-Paranthesis' then
                return token
            when 'SIP-To-Right-Paranthesis','SIP-Contact-Right-Paranthesis' then
                return token
            else
                return word
        end
    end

    def getAllMsgEl(orgMsg)
        temp = orgMsg.scan(/\{\?(.+?)\?/)
        templist = []
        for i in 0..temp.length-1 do
            templist << temp[i][0]
        end
        return templist
    end

end

# a = MalfSipMsgGen.new(path="test", fName="./REGISTER.txt", msgelement=['SIP-Request-URI-CRLF'], msgElCount=5)
a = MalfSipMsgGen.new()
a.generateAllMalformedMsg()

