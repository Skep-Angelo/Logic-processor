newfile1 = open("DataLtspice/Bit1.txt", "w")
newfile2 = open("DataLtspice/Bit2.txt", "w")
newfile3 = open("DataLtspice/Bit3.txt", "w")
newfile4 = open("DataLtspice/Bit4.txt", "w")
newfile5 = open("DataLtspice/Bit5.txt", "w")
newfile6 = open("DataLtspice/Bit6.txt", "w")
newfile7 = open("DataLtspice/Bit7.txt", "w")
newfile8 = open("DataLtspice/Bit8.txt", "w")
newfile9 = open("DataLtspice/Bit9.txt", "w")
newfile10 = open("DataLtspice/Bit10.txt", "w")
newfile11 = open("DataLtspice/Bit11.txt", "w")
newfile12 = open("DataLtspice/Bit12.txt", "w")
newfile13 = open("DataLtspice/Bit13.txt", "w")
newfile14 = open("DataLtspice/Bit14.txt", "w")
newfile15 = open("DataLtspice/Bit15.txt", "w")
newfile16 = open("DataLtspice/Bit16.txt", "w")
newfile17 = open("DataLtspice/Bit17.txt", "w")
newfile18 = open("DataLtspice/Bit18.txt", "w")
newfile19 = open("DataLtspice/Bit19.txt", "w")
newfile20 = open("DataLtspice/Bit20.txt", "w")
newfile21 = open("DataLtspice/Bit21.txt", "w")
newfile22 = open("DataLtspice/Bit22.txt", "w")
newfile23 = open("DataLtspice/Bit23.txt", "w")
newfile24 = open("DataLtspice/Bit24.txt", "w")
newfile25 = open("DataLtspice/Bit25.txt", "w")

with open('output.bin', 'rb') as f:
    time = 0
    while True:
        data = f.read(4)
        if not data:
            break

        instr = int.from_bytes(data, byteorder="little")

        cond = (instr >> 20) & 0x1F
        opcode = (instr >> 15) & 0x1F
        src1 = (instr >> 10) & 0x1F
        src2 = (instr >> 5) & 0x1F
        dest = instr & 0x1F

        binary_str = format(instr & 0x1FFFFFF, "025b")

        control = list(format(cond, '05b'))
        command = list(format(opcode, '05b'))
        ram1 = list(format(src1, '05b'))
        ram2 = list(format(src2, '05b'))
        ram3 = list(format(cond, '05b'))

        print(f"{cond:05b} {opcode:05b} {src1:05b} {src2:05b} {dest:05b}")
        
        newfile1.write(f"{time}\t{control[0]}\n")
        newfile2.write(f"{time}\t{control[1]}\n")
        newfile3.write(f"{time}\t{control[2]}\n")
        newfile4.write(f"{time}\t{control[3]}\n")
        newfile5.write(f"{time}\t{control[4]}\n")
        newfile6.write(f"{time}\t{command[0]}\n")
        newfile7.write(f"{time}\t{command[1]}\n")
        newfile8.write(f"{time}\t{command[2]}\n")
        newfile9.write(f"{time}\t{command[3]}\n")
        newfile10.write(f"{time}\t{command[4]}\n")
        newfile11.write(f"{time}\t{ram1[0]}\n")
        newfile12.write(f"{time}\t{ram1[1]}\n")
        newfile13.write(f"{time}\t{ram1[2]}\n")
        newfile14.write(f"{time}\t{ram1[3]}\n")
        newfile15.write(f"{time}\t{ram1[4]}\n")
        newfile16.write(f"{time}\t{ram2[0]}\n")
        newfile17.write(f"{time}\t{ram2[1]}\n")
        newfile18.write(f"{time}\t{ram2[2]}\n")
        newfile19.write(f"{time}\t{ram2[3]}\n")
        newfile20.write(f"{time}\t{ram2[4]}\n")
        newfile21.write(f"{time}\t{ram3[0]}\n")
        newfile22.write(f"{time}\t{ram3[1]}\n")
        newfile23.write(f"{time}\t{ram3[2]}\n")
        newfile24.write(f"{time}\t{ram3[3]}\n")
        newfile25.write(f"{time}\t{ram3[4]}\n")
        time += 1
        print("-"*50)

