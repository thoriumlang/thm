---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by cpollet.
--- DateTime: 16.12.21 20:31
---

Lexer = {}

local function enum(tbl)
    for i = 1, #tbl do
        tbl[tbl[i]] = i
    end
    return tbl
end

local function lookupify(src)
    list = {}
    for i = 1, src:len() do
        list[src:sub(i, i)] = true
    end
    return list
end

TokenType = enum {
    "EOF",
    "LPAR",
    "RPAR",
    "COMMA",
    "ID",
    "TEXT",
    "KEYWORD",
    "NUMBER",
    "REGISTER",
    "WORD",
    "BYTE",
    "FLAG",
    "WHITESPACE",
}

function Lexer:new(text)
    local lexer = {
        text = text;
        start = 1;
        pos = 1;
        line = 1;
        column = 1;
    }
    setmetatable(lexer, { __index = Lexer })
    return lexer
end

local chars = {
    whitespace = lookupify(" \n\t\r"),
    digit = lookupify("0123456789"),
    wordLetter = lookupify("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"),
    flags = lookupify("nz"),
}

local keyword = enum {
    "op",
    "reg",
    "effect",
    "flags",
    "args",
    "comment",
    "cond",
    "code",
    "index"
}

function Lexer:next()
    local function look(delta)
        delta = self.pos + (delta or 0)
        return self.text:sub(delta, delta)
    end

    local function eat(count)
        count = (count or 1)
        self.pos = self.pos + count
        self.column = self.column + count
    end

    local function get()
        eat()
        return look(-1)
    end

    local function readWhitespaces()
        while true do
            if look() == '\n' then
                self.pos = self.pos + 1
                self.line = self.line + 1
                self.column = 1
            elseif chars.whitespace[look()] then
                eat()
            else
                break
            end
        end
        return self.text:sub(self.start, self.pos - 1)
    end

    local function skipBlockComment()
        while true do
            if look() == "#" and look(1) == "#" and look(2) == "#" then
                eat(3)
                break
            end
            if chars.whitespace[look()] then
                readWhitespaces()
            else
                eat()
            end
        end
        self.start = self.pos
    end

    local function readWord()
        while true do
            if chars.wordLetter[look()] then
                eat()
            else
                break ;
            end
        end
        return self.text:sub(self.start, self.pos - 1)
    end

    local function readNumber()
        while true do
            if chars.digit[look()] then
                eat()
            else
                break ;
            end
        end
        return self.text:sub(self.start, self.pos - 1)
    end

    local function isMnemonic(s)
        return s == string.upper(s)
    end

    local function readText()
        local level = 0
        while true do
            if look() == "" then
                return self.text:sub(self.start, self.pos - 1)
            elseif look() == "(" then
                eat()
                level = level + 1
            elseif look() == ")" and level == 0 then
                break
            elseif look() == ")" then
                eat()
                level = level - 1
            else
                eat()
            end
        end
        return self.text:sub(self.start, self.pos - 1)
    end

    local function makeToken(type, value)
        value = (value or self.text:sub(self.start, self.pos - 1))
        local token = {
            type = type,
            line = self.line,
            column = self.column - #value,
            value = value
        }
        self.start = self.pos
        return token
    end

    while true do
        local whitespaces = readWhitespaces()

        if whitespaces ~= "" then
            return makeToken(TokenType.WHITESPACE)
        end

        local char = get()

        if char == "" then
            return makeToken(TokenType.EOF)
        end
        if char == "#" and look() == "#" and look(1) == "#" then
            eat(2)
            skipBlockComment()
        elseif char == "#" and look() then
            while look() ~= "\n" do
                eat()
            end
        elseif char == "(" then
            return makeToken(TokenType.LPAR)
        elseif char == ")" then
            return makeToken(TokenType.RPAR)
        elseif char == "," then
            return makeToken(TokenType.COMMA)
        elseif chars.digit[char] then
            return makeToken(TokenType.NUMBER, readNumber())
        elseif char == "r" and chars.digit[look()] then
            return makeToken(TokenType.REGISTER, readNumber())
        elseif char == "w" and chars.digit[look()] then
            return makeToken(TokenType.WORD, readNumber())
        elseif char == "b" and chars.digit[look()] then
            return makeToken(TokenType.BYTE, readNumber())
        elseif chars.flags[char] then
            return makeToken(TokenType.FLAG)
        elseif chars.wordLetter[char] then
            local word = readWord()
            if keyword[word] then
                return makeToken(TokenType.KEYWORD)
            end
            if isMnemonic(word) then
                return makeToken(TokenType.ID)
            end
            return makeToken(TokenType.TEXT, readText())
        else
            return makeToken(TokenType.TEXT, readText())
        end
    end
end