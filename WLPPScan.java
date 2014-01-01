import java.util.*;
import java.math.*;

/** A sample main class demonstrating the use of the Lexer.
 * This main class just outputs each line in the input, followed by
 * the tokens returned by the lexer for that line.
 *
 * @version 071011.0
 */
public class WLPPScan {
    public static final void main(String[] args) {
        new WLPPScan().run();
    }

    private Lexer lexer = new Lexer();

    private void run() {
        Scanner in = new Scanner(System.in);
        while(in.hasNextLine()) {
            String line = in.nextLine();

            // Scan the line into an array of tokens.
            Token[] tokens;
            tokens = lexer.scan(line);

            // Print the tokens produced by the scanner
            for( int i = 0; i < tokens.length; i++ ) {
              if(tokens[i].lexeme.equals("return"))
                System.out.println(Kind.RETURN + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("if"))
                System.out.println(Kind.IF + " " + tokens[i].lexeme + " ");
			  else if(tokens[i].lexeme.equals("println"))
                System.out.println(Kind.PRINTLN + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("wain"))
                System.out.println(Kind.WAIN + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("int"))
                System.out.println(Kind.INT + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("else"))
                System.out.println(Kind.ELSE + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("while"))
                System.out.println(Kind.WHILE + " " + tokens[i].lexeme + " ");
              
              else if(tokens[i].lexeme.equals("new"))
                System.out.println(Kind.NEW + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("delete"))
                System.out.println(Kind.DELETE + " " + tokens[i].lexeme + " ");
              else if(tokens[i].lexeme.equals("NULL"))
                System.out.println(Kind.NULL + " " + tokens[i].lexeme + " ");
              else
                System.out.println(tokens[i].kind + " " + tokens[i].lexeme + " ");
            }
        }
    }
}

/** The various kinds of tokens. */
enum Kind {
    ID,         // Opcode or identifier (use of a label)
    NUM,        //integer numbers not zero
    ZERO,       //zero
    LBRACE,
    RBRACE,
    RETURN,
	LBRACK,
    RBRACK,
    AMP,
    NULL,
    COMMA,
    IF,
    ELSE,
    WHILE,
    PRINTLN,
    WAIN,
    BECOMES,
    INT,
	PLUS,
    MINUS,
    STAR,
    SLASH,
    PCT,
    SEMI,
    NEW,
    DELETE,
	EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
     // Comma
    LPAREN,     // (
    RPAREN,     // )
    WHITESPACE; // Whitespace
}

/** Representation of a token. */
class Token {
    public Kind kind;     // The kind of token.
    public String lexeme; // String representation of the actual token in the
                          // source code.

    public Token(Kind kind, String lexeme) {
        this.kind = kind;
        this.lexeme = lexeme;
    }
    public String toString() {
        return kind+" {"+lexeme+"}";
    }
    /** Returns an integer representation of the token. For tokens of kind
     * INT (decimal integer constant) and HEXINT (hexadecimal integer
     * constant), returns the integer constant. For tokens of kind
     * REGISTER, returns the register number.
     */
    /*public int toInt() {
        if(kind == Kind.INT) return parseLiteral(lexeme, 10, 32);
        else if(kind == Kind.HEXINT) return parseLiteral(lexeme.substring(2), 16, 32);
        else if(kind == Kind.REGISTER) return parseLiteral(lexeme.substring(1), 10, 5);
        else {
            System.err.println("ERROR in to-int conversion.");
            System.exit(1);
            return 0;
        }
    }
    private int parseLiteral(String s, int base, int bits) {
        BigInteger x = new BigInteger(s, base);
        if(x.signum() > 0) {
            if(x.bitLength() > bits) {
                System.err.println("ERROR in parsing: constant out of range: "+s);
                System.exit(1);
            }
        } else if(x.signum() < 0) {
            if(x.negate().bitLength() > bits-1
            && x.negate().subtract(new BigInteger("1")).bitLength() > bits-1) {
                System.err.println("ERROR in parsing: constant out of range: "+s);
                System.exit(1);
            }
        }
        return (int) (x.longValue() & ((1L << bits) - 1));
    }*/
}

/** Lexer -- reads an input line, and partitions it into a list of tokens. */
class Lexer {
    public Lexer() {
        CharSet whitespace = new Chars("\t\n\r ");
        CharSet letters = new Chars(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
        CharSet lettersDigits = new Chars(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
        CharSet digits = new Chars("0123456789");
        CharSet hexDigits = new Chars("0123456789ABCDEFabcdef");
        CharSet oneToNine = new Chars("123456789");
        CharSet all = new AllChars(); 

        table = new Transition[] {
                new Transition(State.START, whitespace, State.WHITESPACE),
                new Transition(State.START, letters, State.ID),
                new Transition(State.ID, lettersDigits, State.ID),
                new Transition(State.START, oneToNine, State.NUM),
                new Transition(State.START, new Chars("0"), State.ZERO),
                new Transition(State.NUM, digits, State.NUM),
                new Transition(State.START, new Chars("("), State.LPAREN),
                new Transition(State.START, new Chars(")"), State.RPAREN),
                new Transition(State.START, new Chars("{"), State.LBRACE),
                new Transition(State.START, new Chars("}"), State.RBRACE),
                new Transition(State.START, new Chars("="), State.BECOMES),
				new Transition(State.START, new Chars("+"), State.PLUS),
                new Transition(State.START, new Chars("-"), State.MINUS),
                new Transition(State.START, new Chars("*"), State.STAR),
                new Transition(State.START, new Chars("/"), State.SLASH),
                new Transition(State.START, new Chars("%"), State.PCT),
                new Transition(State.START, new Chars(","), State.COMMA),
                new Transition(State.START, new Chars(";"), State.SEMI),
                new Transition(State.START, new Chars("["), State.LBRACK),
                new Transition(State.START, new Chars("]"), State.RBRACK),
                new Transition(State.START, new Chars("&"), State.AMP),
                new Transition(State.BECOMES, new Chars("="), State.EQ),
                new Transition(State.START, new Chars("!"), State.NOT),
                new Transition(State.NOT, new Chars("="), State.NE),
                new Transition(State.START, new Chars("<"), State.LT),
                new Transition(State.START, new Chars(">"), State.GT),
                new Transition(State.LT, new Chars("="), State.LE),
                new Transition(State.GT, new Chars("="), State.GE),
                new Transition(State.SLASH, new Chars("/"), State.COMMENT),
                new Transition(State.COMMENT, all, State.COMMENT),
                new Transition(State.ZERO, lettersDigits, State.ERROR)
        };
    }
    /** Partitions the line passed in as input into an array of tokens.
     * The array of tokens is returned.
     */
    public Token[] scan( String input ) {
        List<Token> ret = new ArrayList<Token>();
        if(input.length() == 0) return new Token[0];
        int i = 0;
        int startIndex = 0;
        State state = State.START;
        while(true) {
            Transition t = null;
            if(i < input.length()) t = findTransition(state, input.charAt(i));
            if(t == null) {
                // no more transitions possible
                if(!state.isFinal()) {
                    System.err.println("ERROR in lexing after reading "+input.substring(0, i));
                    System.exit(1);
                }
                if( state.kind != Kind.WHITESPACE ) {
                    ret.add(new Token(state.kind,
                                input.substring(startIndex, i)));
                }
                startIndex = i;
                state = State.START;
                if(i >= input.length()) break;
            } else {
                state = t.toState;
                i++;
            }
        }
        return ret.toArray(new Token[ret.size()]);
    }

    ///////////////////////////////////////////////////////////////
    // END OF PUBLIC METHODS
    ///////////////////////////////////////////////////////////////

    private Transition findTransition(State state, char c) {
        for( int j = 0; j < table.length; j++ ) {
            Transition t = table[j];
            if(t.fromState == state && t.chars.contains(c)) {
                return t;
            }
        }
        return null;
    }

    private static enum State {
        START(null),
        ERROR(null),
        ID(Kind.ID),
        NUM(Kind.NUM),
		ZERO(Kind.NUM),
        LBRACE(Kind.LBRACE),
        RBRACE(Kind.RBRACE),
        BECOMES(Kind.BECOMES),
		PLUS(Kind.PLUS),
        MINUS(Kind.MINUS),
        STAR(Kind.STAR),
        SLASH(Kind.SLASH),
        PCT(Kind.PCT),
        NOT(null),
        SEMI(Kind.SEMI),
        LBRACK(Kind.LBRACK),
        RBRACK(Kind.RBRACK),
        AMP(Kind.AMP),
        COMMA(Kind.COMMA),
        LPAREN(Kind.LPAREN),
        RPAREN(Kind.RPAREN),
		EQ(Kind.EQ),
        NE(Kind.NE),
        LT(Kind.LT),
        GT(Kind.GT),
        LE(Kind.LE),
        GE(Kind.GE),
        
        COMMENT(Kind.WHITESPACE),
        WHITESPACE(Kind.WHITESPACE);
        State(Kind kind) {
            this.kind = kind;
        }
        Kind kind;
        boolean isFinal() {
            return kind != null;
        }
    }

    private interface CharSet {
        public boolean contains(char newC);
    }
    private class Chars implements CharSet {
        private String chars;
        public Chars(String chars) { this.chars = chars; }
        public boolean contains(char newC) {
            return chars.indexOf(newC) >= 0;
        }
    }
    private class AllChars implements CharSet {
        public boolean contains(char newC) {
            return true;
        }
    }

    private class Transition {
        State fromState;
        CharSet chars;
        State toState;
        Transition(State fromState, CharSet chars, State toState) {
            this.fromState = fromState;
            this.chars = chars;
            this.toState = toState;
        }
    }
    private Transition[] table;
}


