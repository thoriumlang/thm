#!/usr/bin/env lua

---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by cpollet.
--- DateTime: 16.12.21 20:32
---

local requireRel
if arg and arg[0] then
    package.path = arg[0]:match("(.-)[^\\/]+$") .. "?.lua;" .. package.path
    requireRel = require
elseif ... then
    local d = (...):match("(.-)[^%.]+$")
    function requireRel(module)
        return require(d .. module)
    end
end

function enum(tbl)
    for i = 1, #tbl do
        tbl[tbl[i]] = i
    end
    return tbl
end

function map(f, t)
    local t1 = {}
    local t_len = #t
    for i = 1, t_len do
        t1[i] = f(t[i])
    end
    return t1
end

function sum(t)
    local r = 0
    local t_len = #t

    for i = 1, t_len do
        r = r + t[i]
    end

    return r
end

local function roundTo(a, b)
    return math.ceil(a / b) * b
end

function string:rpad(l, c)
    return self .. string.rep(c or ' ', l - #self)
end

function table.copy(t)
    local u = { }
    for k, v in pairs(t) do
        u[k] = v
    end
    return u
end

requireRel("ths-lexer")
requireRel("ths-parser")

local function read_file(path)
    local file = io.open(path, "rb") -- r read mode and b binary mode
    if not file then
        return nil
    end
    local content = file:read "*a" -- *a or *all reads the whole file
    file:close()
    return content
end

local lexer = Lexer:new(read_file(arg[1]))
local parser = Parser:new(lexer)
local ast = parser:parse()

for i = 1, #ast do
    if ast[i].code > 255 then
        io.write("Illegal code " .. ast[i].code .. " for " .. ast[i].toString())
    end
end

--for i = 1, #ast do
--    io.write(ast[i]:toString() .. "\n")
--end

local instructions = {}
for _, v in ipairs(ast) do
    instructions[v.code] = v
end

-- Generate src/vm/ops_array.h ------------------
local file = io.open("src/vm/ops_array.h", "w")
io.output(file)
io.write("// See https://github.com/thoriumlang/thm/wiki/Instructions\n")
io.write("#include \"ops.h\"\n")
io.write("op_ptr ops[256] = {\n")
for i = 0, 255 do
    if instructions[i] == nil then
        io.write("        NULL, // 0x" .. string.format("%x", i) .. "\n")
    else
        local suffix = ""
        if instructions[i].args:len() > 0 then
            suffix = "_" .. table.concat(map(Arg.typeLetter, instructions[i].args), "")
        end
        io.write("        &op_" .. instructions[i].name:lower() .. suffix:lower() .. ", // 0x" .. string.format("%02x", i) .. "\n")
    end
end
io.write("};\n")
io.write("enum ops_list {\n")
for i = 0, 255 do
    if instructions[i] == nil then
        io.write("        _" .. i .. ", // 0x" .. string.format("%x", i) .. "\n")
    else
        local suffix = ""
        if instructions[i].args:len() > 0 then
            suffix = "_" .. table.concat(map(Arg.typeLetter, instructions[i].args), "")
        end
        io.write("        " .. instructions[i].name:upper() .. suffix:upper() .. ", // 0x" .. string.format("%02x", i) .. "\n")
    end
end
io.write("};\n")
io.write("char *ops_name[256] = {\n")
for i = 0, 255 do
    if instructions[i] == nil then
        io.write("        \"_    \", // 0x" .. string.format("%x", i) .. "\n")
    else
        io.write("        \"" .. instructions[i].name:upper():rpad(5, " ") .. "\", // 0x" .. string.format("%02x", i) .. "\n")
    end
end
io.write("};\n")
io.close(file)

-- Generate src/vm/ops_array --------------------
local file = io.open("src/asm/op.rs", "w")
io.output(file)
io.write("/// See https://github.com/thoriumlang/thm/wiki/Instructions\n")
io.write("#[derive(Debug, PartialEq, Clone, Copy)]\n")
io.write("pub enum Op {\n")
for _, v in ipairs(ast) do
    local suffix = ""
    if v.args:len() > 0 then
        suffix = table.concat(map(Arg.typeLetter, v.args), "")
    end
    io.write("    " .. v.name:sub(1, 1):upper() .. v.name:sub(2):lower() .. suffix:upper() .. " = " .. v.code .. ", // 0x" .. string.format("%02x", v.code) .. "\n")
end
io.write("}\n")
io.write("impl Op {\n")
io.write("   pub fn length(&self) -> u8 {\n")
io.write("        match self {\n")
for _, v in ipairs(ast) do
    local suffix = ""
    if v.args:len() > 0 then
        suffix = table.concat(map(Arg.typeLetter, v.args), "")
    end
    io.write("            Op::" .. v.name:sub(1, 1):upper() .. v.name:sub(2):lower() .. suffix:upper() .. " => " .. roundTo(v:size(), 4) .. ",\n");
end
io.write("       }\n");
io.write("    }\n");
io.write("\n");
io.write("    pub fn bytecode(&self) -> u8 {\n");
io.write("       *self as u8\n");
io.write("    }\n");
io.write("}\n");
io.write("\n");
io.write("impl From<u8> for Op {\n");
io.write("    fn from(v: u8) -> Self {\n");
io.write("        match v {\n");
for _, v in ipairs(ast) do
    local suffix = ""
    if v.args:len() > 0 then
        suffix = table.concat(map(Arg.typeLetter, v.args), "")
    end
    io.write("            " .. v.code .. " => Self::" .. v.name:sub(1, 1):upper() .. v.name:sub(2):lower() .. suffix:upper() .. ",\n")
end
io.write("            _ => Self::Panic,\n")
io.write("        }\n")
io.write("    }\n")
io.write("}\n")
io.write("\n")
io.write("\n")
io.close(file)