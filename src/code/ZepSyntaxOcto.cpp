#include "ZepSyntaxOcto.h"

#include <unordered_set>
#include <spdlog/spdlog.h>

namespace dorito {

  static std::unordered_set<std::string> octo_keywords = {
      "loop", "again", "while", "begin", "if", "then", "else", "end", "return", ";"
  };

  static std::unordered_set<std::string> octo_identifiers = {
      "clear", "bcd", "save", "load", "sprite", "jump", "jump0",
      "delay", "buzzer", "hires", "lores", "scroll-down",
      "scroll-left", "scroll-right", "scroll-up", "bighex", "exit",
      "saveflags", "loadflags", "long", "plane", "pitch", "audio",
      "sin", "cos", "tan", "exp", "log", "abs", "sqrt", "sign",
      "ceil", "floor", "@", "strlen", "pow", "min", "max", "HERE",
      "PI"
  };

  ZepSyntaxOcto::ZepSyntaxOcto(Zep::ZepBuffer &buffer)
      : Zep::ZepSyntax(buffer, octo_keywords, octo_identifiers) {}

  void ZepSyntaxOcto::registerSyntax(std::unique_ptr<Zep::ZepEditor> &editor) {
    editor->RegisterSyntaxFactory({".o8"}, Zep::SyntaxProvider{"Octo", tSyntaxFactory([](Zep::ZepBuffer *pBuffer) {
      return std::make_shared<ZepSyntaxOcto>(*pBuffer);
    })});
  }

  void ZepSyntaxOcto::UpdateSyntax() {
    auto &buffer = m_buffer.GetWorkingBuffer();
    auto itrCurrent = buffer.begin();
    auto itrEnd = buffer.begin() + m_targetChar;
    auto bufEnd = buffer.end();

    assert(std::distance(itrCurrent, itrEnd) < int(m_syntax.size()));
    assert(m_syntax.size() == buffer.size());

    static const std::string delim(" \r\v\n\t+*/&|^!~%#$<>=,(){}[]");
    static const std::string whiteSpace(" \t\v\r\n");
    static const std::string delimWithDot = delim + ".";

    auto mark = [&](GapBuffer<uint8_t>::const_iterator itrA, GapBuffer<uint8_t>::const_iterator itrB,
                    Zep::ThemeColor type, Zep::ThemeColor background) {
      std::fill(m_syntax.begin() + (itrA - buffer.begin()), m_syntax.begin() + (itrB - buffer.begin()),
                Zep::SyntaxData{type, background});
    };

    m_processedChar = long(itrCurrent - buffer.begin());

    while (itrCurrent < itrEnd) {
      if (m_stop) {
        return;
      }

      auto ch = *itrCurrent;

      auto peek = [&](unsigned inc) -> decltype(ch) {
        auto itr = itrCurrent + inc;
        return itr < buffer.end() ? *itr : '\n';
      };

      if (whiteSpace.find_first_of(ch) != std::string::npos) {
        mark(itrCurrent, itrCurrent + 1, Zep::ThemeColor::Whitespace, Zep::ThemeColor::None);
        itrCurrent++;
        continue;
      }

      if (ch == '#') {
        auto commentLeft = itrCurrent;
        auto commentRight = commentLeft;

        auto cch = *commentLeft;
        bool early = false;
        while (cch != '\n' && cch != '\r') {
          if (commentRight >= itrEnd) {
            early = true;
            break;
          }

          cch = *commentRight;

          commentRight++;
        }

        mark(commentLeft, commentRight, Zep::ThemeColor::Info, Zep::ThemeColor::None);

        if (early) {
          itrCurrent++;
          continue;
        }

        if (commentRight >= itrEnd)
          break;

        itrCurrent = commentRight;
        continue;
      }

      if (ch == ':') {
        switch (peek(1)) {
          case '=':
          case '\n':
            itrCurrent++;
            continue;

          case ' ': {
            // Label
            auto labelEnd = itrCurrent;
            uint8_t wsCount = 0;

            bool early = false;
            while (wsCount < 2) {
              if (labelEnd >= itrEnd) {
                early = true;
                break;
              }

              auto lch = *labelEnd++;

              if (lch == ' ' || lch == '\n' || lch == '\r' || lch == '\t' || lch == '\v')
                wsCount++;
            }

            if (early) {
              itrCurrent++;
              continue;
            }

            mark(itrCurrent, labelEnd, Zep::ThemeColor::TabActive, Zep::ThemeColor::None);

            itrCurrent = labelEnd;
            continue;
          }

          default: {
            // Psuedo Op
            auto opEnd = itrCurrent;
            uint8_t wsCount = 0;

            bool early = false;

            while (wsCount < 1) {
              if (opEnd >= itrEnd) {
                early = true;
                break;
              }

              auto lch = *opEnd++;

              if (lch == ' ' || lch == '\n' || lch == '\r' || lch == '\t' || lch == '\v')
                wsCount++;
            }

            if (early) {
              itrCurrent++;
              continue;
            }

            mark(itrCurrent, opEnd, Zep::ThemeColor::Identifier, Zep::ThemeColor::None);

            itrCurrent = opEnd;
            continue;
          }
        }
      }

      if (ch == '"') {
        if (peek(1) == '\n') {
          itrCurrent++;
          continue;
        }

        auto itrLeft = itrCurrent;
        auto itrRight = itrLeft;

        itrRight++;

        while (itrRight < itrEnd) {
          if (*itrRight == '"') {
            itrRight++;
            mark(itrLeft, itrRight, Zep::ThemeColor::String, Zep::ThemeColor::None);

            itrCurrent = itrRight;
            break;
          }

          if (itrRight < (itrEnd - 1)) {
            auto itrNext = itrRight + 1;
            if (*itrRight == '\\' && *itrNext == ch) {
              itrRight++;
            }
          }

          itrRight++;
        }

        if (itrRight >= itrEnd) {
          break;
        }

        continue;
      }

      auto firstChar = buffer.find_first_not_of(itrCurrent, bufEnd, delimWithDot.begin(), delimWithDot.end());

      if (firstChar == bufEnd)
        break;

      auto lastChar = buffer.find_first_of(firstChar, bufEnd, delimWithDot.begin(), delimWithDot.end());

      assert(lastChar >= firstChar);

      auto token = std::string(firstChar, lastChar);

      if (m_keywords.find(token) != m_keywords.end()) {
        mark(firstChar, lastChar, Zep::ThemeColor::Keyword, Zep::ThemeColor::None);
        itrCurrent = lastChar;
        continue;
      } else if (m_identifiers.find(token) != m_identifiers.end()) {
        mark(firstChar, lastChar, Zep::ThemeColor::Identifier, Zep::ThemeColor::None);
        itrCurrent = lastChar;
        continue;
      } else {
        mark(firstChar, lastChar, Zep::ThemeColor::Normal, Zep::ThemeColor::None);
        //itrCurrent = lastChar;
      }

      auto parseHex = [&](decltype(firstChar) itr, decltype(firstChar) &last) -> bool {
        auto first = itr;
        static const std::string hexa("0123456789abcdefABCDEF");
        while (itr < buffer.end()) {
          auto ch = *itr;
          if (delim.find_first_of(ch) != std::string::npos) {
            break;
          }
          if (hexa.find_first_of(ch) == std::string::npos) {
            return false;
          }
          itr++;
        }

        last = itr;
        return itr != first;
      };

      auto parseBinary = [&](decltype(firstChar) itr, decltype(firstChar) &last) -> bool {
        auto first = itr;
        static const std::string binary("01");
        while (itr < buffer.end()) {
          auto ch = *itr;
          if (delim.find_first_of(ch) != std::string::npos) {
            break;
          }
          if (binary.find_first_of(ch) == std::string::npos) {
            return false;
          }
          itr++;
        }

        last = itr;
        return itr != first;
      };

      auto parseNumber = [&](decltype(firstChar) itr, decltype(firstChar) &last) -> bool {
        auto first = itr;
        static const std::string numbers("0123456789");

        bool gotSomething = false;

        while (itr < buffer.end()) {
          auto ch = *itr;
          if (delim.find_first_of(ch) != std::string::npos) {
            break;
          }

          if (numbers.find_first_of(ch) != std::string::npos) {
            gotSomething = true;
          } else {
            return false;
          }
          itr++;
        }

        last = itr;
        return itr != first && gotSomething;
      };

      if (std::isdigit(*firstChar)) {
        auto itrNum = firstChar;
        auto last = lastChar;
        bool parsed;

        if (*firstChar == '0') {
          itrNum++;

          switch (*itrNum) {
            case 'x':
            case 'X':
              parsed = parseHex(firstChar + 2, last);
              break;

            case 'b':
            case 'B':
              parsed = parseBinary(firstChar + 2, last);
              break;

            default:
              parsed = parseNumber(firstChar, last);
              break;
          }
        } else {
          parsed = parseNumber(firstChar, last);
        }

        if (parsed) {
          mark(firstChar, last, Zep::ThemeColor::Number, Zep::ThemeColor::None);
        }
      }

      itrCurrent++;
    }

    m_targetChar = long(0);
    m_processedChar = long(buffer.size() - 1);
  }
} // dorito