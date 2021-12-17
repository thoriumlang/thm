---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by cpollet.
--- DateTime: 17.12.21 08:48
---

-- Arg ------------------------------------------
ArgType = enum {
    "Register",
    "Word",
    "Byte",
    "CsRelativeAddress",
    "AbsoluteAddress",
}
Arg = {}
function Arg:new(type, value)
    local data = {
        type = type,
        value = value,
    }
    setmetatable(data, { __index = Arg })
    return data
end
function Arg:register(value)
    return Arg:new(ArgType.Register, value)
end
function Arg:word(value)
    return Arg:new(ArgType.Word, value)
end
function Arg:byte(value)
    return Arg:new(ArgType.Byte, value)
end
function Arg:csRelativeAddress(value)
    return Arg:new(ArgType.CsRelativeAddress, value)
end
function Arg:absoluteAddress(value)
    return Arg:new(ArgType.AbsoluteAddress, value)
end
function Arg:toString()
    if self.type == ArgType.Register then
        return "r" .. self.value
    elseif self.type == ArgType.Word then
        return "w" .. self.value
    elseif self.type == ArgType.Byte then
        return "b" .. self.value
    elseif self.type == ArgType.CsRelativeAddress then
        return "@" .. self.value
    elseif self.type == ArgType.AbsoluteAddress then
        return "&" .. self.value
    end
end
function Arg:size()
    if self.type == ArgType.Register then
        return 1
    elseif self.type == ArgType.Byte then
        return 1
    elseif self.type == ArgType.Word then
        return 4
    elseif self.type == ArgType.CsRelativeAddress then
        return 4
    elseif self.type == ArgType.AbsoluteAddress then
        return 4
    end
end

-- Args -----------------------------------------
Args = {}
function Args:new(args)
    setmetatable(args, { __index = Args })
    return args
end
function Args:toString()
    return table.concat(map(Arg.toString, self), ", ")
end
function Args:size()
    return sum(map(Arg.size, self))
end

-- Flag -----------------------------------------
Flag = {}
function Flag:new(flag)
    local data = {
        flag = flag
    }
    setmetatable(data, { __index = Flag })
    return data
end
function Flag:toString()
    return self.flag
end

-- Flags ----------------------------------------
Flags = {}
function Flags:new(flags)
    setmetatable(flags, { __index = Flags })
    return flags
end
function Flags:toString()
    return table.concat(map(Flag.toString, self), ", ")
end

-- Node -----------------------------------------
Instruction = {}
function Instruction:new(data)
    setmetatable(data, { __index = Instruction })
    return data
end
function Instruction:toString()
    local string = "Instruction " .. self.name
    if self.abstract then
        string = string .. " (abstract)"
    else
        string = string .. "\n  Code:   " .. self.code
    end
    string = string .. "\n  Args:   " .. self.args:toString()
    if self.cond ~= "" then
        string = string .. "\n  Cond:   " .. self.cond
    end
    string = string .. "\n  Effect: " .. self.effect
    if self.flags then
        string = string .. "\n  Flags:  " .. self.flags:toString()
    end
    if self.comment ~= "" then
        string = string .. "\n  Comm:   " .. self.comment
    end
    string = string .. "\n  Size:   " .. 1 + self.args:size() .. " bytes"
    return string
end

-- Parser ---------------------------------------
Parser = {}
function Parser:new(lexer)
    local parser = {
        lexer = lexer,
        look = {},
        op = {
            nextCode = 0
        }
    }
    setmetatable(parser, { __index = Parser })
    return parser
end

function Parser:parse()
    local function printError(expectedType, token)
        if type(expectedType) == "number" then
            io.write("Expected " .. TokenType[expectedType])
        elseif type(expectedType) == "table" then
            io.write("Expected any of " .. table.concat(map(TokenType.toString, expectedType), ", "))
        end
        io.write(" at " .. token.line .. ":" .. token.column
                .. ", got " .. TokenType[token.type] .. "\n")
        os.exit(1)
    end

    local function look(k, type, keepWhitespaces)
        k = (k or 1)
        type = (type or nil)
        skipWhitespaces = not keepWhitespaces

        local i = 1
        while true do
            if #self.look < i then
                self.look[i] = self.lexer:next()
            end
            while self.look[i].type == TokenType.WHITESPACE and skipWhitespaces do
                i = i + 1
                k = k + 1
                if #self.look < i then
                    self.look[i] = self.lexer:next()
                end
                if self.look[i].type ~= TokenType.WHITESPACE then
                    break
                end
            end
            if i == k then
                if type and self.look[i].type ~= type then
                    printError(type, self.look[i])
                end
                return self.look[i]
            end
            i = i + 1
        end
    end

    local function _next()
        local token = table.remove(self.look, 1)
        if token == nil then
            token = self.lexer:next()
        end
        return token
    end

    local function next(type, keepWhitespaces)
        type = (type or nil)
        skipWhitespaces = not keepWhitespaces

        local token = _next()
        while skipWhitespaces do
            if token.type ~= TokenType.WHITESPACE then
                break
            end
            token = _next()
        end

        if type and token.type ~= type then
            printError(type, token)
        end

        return token
    end

    -- returns an array of Arg
    local function parseArgs()
        local args = {}
        local token = nil
        while true do
            token = look()
            if token.type == TokenType.RPAR then
                break
            elseif token.type == TokenType.COMMA then
                if #args == 0 then
                    printError({ TokenType.REGISTER, TokenType.WORD, TokenType.BYTE, TokenType.ABSOLUTE_ADDRESS, TokenType.CS_ADDRESS }, token)
                end
                next()
            elseif token.type == TokenType.REGISTER then
                table.insert(args, Arg:register(token.value:sub(2)))
                next()
            elseif token.type == TokenType.WORD then
                table.insert(args, Arg:word(token.value:sub(2)))
                next()
            elseif token.type == TokenType.BYTE then
                table.insert(args, Arg:byte(token.value:sub(2)))
                next()
            elseif token.type == TokenType.CS_ADDRESS then
                table.insert(args, Arg:csRelativeAddress(token.value:sub(2)))
                next()
            elseif token.type == TokenType.ABSOLUTE_ADDRESS then
                table.insert(args, Arg:absoluteAddress(token.value:sub(2)))
                next()
            else
                print(token.value)
                printError({ TokenType.REGISTER, TokenType.WORD, TokenType.BYTE, TokenType.ABSOLUTE_ADDRESS, TokenType.CS_ADDRESS }, token)
            end
        end
        if #args == 0 then
            io.write("Expecting at least one argument at " .. token.line .. ":" .. token.column .. "\n")
            os.exit(1)
        end
        return args
    end

    local function parseText()
        local token = look()
        if token.type == TokenType.TEXT then
            return next(TokenType.TEXT).value
        end

        local text = ""
        while true do
            if token.type == TokenType.RPAR then
                break
            end
            text = text .. next(nil, true).value
            token = look(1, nil, true)
        end

        if text == "" then
            io.write("Expected some text at " .. token.line .. ":" .. token.column .. "\n")
            os.exit(1)
        end

        return text:gsub("^%s*(.-)%s*$", "%1")
    end

    local function parseFlags()
        local flags = {}
        local token = nil
        while true do
            token = look()
            if token.type == TokenType.RPAR then
                break
            elseif token.type == TokenType.COMMA then
                if #flags == 0 then
                    printError(TokenType.FLAG, token)
                end
                next()
            elseif token.type == TokenType.FLAG then
                table.insert(flags, Flag:new(token.value))
                next()
            else
                printError({ TokenType.REGISTER, TokenType.WORD, TokenType.BYTE }, token)
            end
        end
        if #flags == 0 then
            io.write("Expecting at least one argument at " .. token.line .. ":" .. token.column .. "\n")
            os.exit(1)
        end
        return flags
    end

    -- returns a table {PairType, any}
    PairType = enum {
        "Code",
        "Args",
        "Flags", -- todo implement
        "Cond",
        "Effect",
        "Comment",
    }
    local function parsePair()
        local token = next(TokenType.LPAR)
        local pair = {
            line = token.line,
            column = token.column,
        }
        local key = next(TokenType.KEYWORD).value
        if key == "code" then
            pair.type = PairType.Code
            pair.value = tonumber(next(TokenType.NUMBER).value)
        elseif key == "args" then
            pair.type = PairType.Args
            pair.value = Args:new(parseArgs())
        elseif key == "effect" then
            pair.type = PairType.Effect
            pair.value = parseText()
        elseif key == "cond" then
            pair.type = PairType.Cond
            pair.value = parseText()
        elseif key == "comment" then
            pair.type = PairType.Comment
            pair.value = parseText()
        elseif key == "flags" then
            pair.type = PairType.Flag
            pair.value = Flags:new(parseFlags())
        end
        next(TokenType.RPAR)

        return pair
    end

    local function parseInstruction(instruction, nested)
        instruction = (instruction or {
            name = next(TokenType.ID).value,
            args = Args:new({}),
            code = nil,
            effect = "n/a",
            cond = "",
            comment = "",
            abstract = false,
        })
        instruction.code = nil

        local instructions = {}

        while true do
            if look().type == TokenType.LPAR then
                if not nested and look(2).type == TokenType.LPAR then
                    instruction.abstract = true
                    next(TokenType.LPAR)
                    table.insert(instructions, parseInstruction(table.copy(instruction), true)[1])
                else
                    local pair = parsePair()
                    if pair.type == PairType.Code then
                        if pair.value < self.op.nextCode then
                            io.write("Code " .. pair.value .. " <= " .. self.op.nextCode .. " at "
                                    .. pair.line .. ":" .. pair.column .. ". Ignoring.\n")
                        else
                            instruction.code = pair.value
                            self.op.nextCode = pair.value
                        end
                    elseif pair.type == PairType.Cond then
                        instruction.cond = pair.value
                    elseif pair.type == PairType.Effect then
                        instruction.effect = pair.value
                    elseif pair.type == PairType.Comment then
                        instruction.comment = pair.value
                    elseif pair.type == PairType.Flag then
                        instruction.flags = pair.value
                    elseif nested and pair.type == PairType.Args then
                        instruction.abstract = false
                        instruction.args = pair.value
                    else
                        io.write(PairType[pair.type] .. " not supported at " .. pair.line .. ":" .. pair.column .. "\n")
                        os.exit(1)
                    end
                end
            else
                next(TokenType.RPAR)
                if not instruction.abstract then
                    if instruction.code == nil then
                        instruction.code = self.op.nextCode
                    end
                    self.op.nextCode = self.op.nextCode + 1
                    table.insert(instructions, Instruction:new(instruction))
                end
                break
            end
        end

        return instructions
    end

    local function parseDirective()
        next(TokenType.LPAR)
        local directive = next(TokenType.KEYWORD).value
        if directive == "op" then
            return parseInstruction()
        end
        next(TokenType.RPAR) -- todo useful
    end

    local directives = {}

    while true do
        local token = look()

        if token.type == TokenType.EOF then
            return directives
        end

        if token.type == TokenType.LPAR then
            local directive = parseDirective()
            if directive ~= nil then
                if type(directive) == "table" then
                    for _, v in ipairs(directive) do
                        table.insert(directives, v)
                    end
                end
            end
        else
            printError(TokenType.LPAR, token)
            return nil
        end
    end
end