/*
 *  src2html
 *      written by H.Tsujimura      14 Jun 2004
 *      All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int src2html( FILE *fp, FILE *gp );

#ifdef  _MSC_VER
#pragma warning ( disable : 4996 )  // for Visual C++ 2005
#endif
int
main( int argc, char *argv[] )
{
    int ret = 0;

    if ( argc <= 1 )
        ret = src2html( stdin, stdout );
    else {
        int     i;
        FILE    *fp;

        for ( i = 1; i < argc; i++ ) {
            fp = fopen( argv[i], "r" );
            if ( fp ) {
                ret += src2html( fp, stdout );
                fclose( fp );
            }
        }
    }

    return ( ret );
}
#ifdef  _MSC_VER
#pragma warning ( default : 4996 )  // for Visual C++ 2005
#endif

static char reservedString[][32] = {
    "#define",
    "#else", "#elif", "#endif", "#error",
    "#ifdef", "#ifndef", "#if", "#import", "#include",
    "#line",
    "#pragma",
    "#undef",
    "__declspec",
    "auto",
    "bool", "break",
    "case", "catch", "char", "class", "const", "continue",
    "default", "defined", "delete", "double", "do",
    "else", "enum", "extern",
    "false", "float", "for", "friend", 
    "if", "int",
    "long",
    "new",
    "public", "private", "protected",
    "register", "return",
    "short", "signed", "sizeof", "static", "struct", "switch",
    "this", "throw", "true", "try", "typedef",
    "union", "unsigned",
    "volatile", "void",
    "while",
    ""
};

#define fputString( p, fp ) \
    if ( *(p) == '<' ) \
        fputs( "&lt;", (fp) ); \
    else if ( *(p) == '>' ) \
        fputs( "&gt;", (fp) ); \
    else if ( *(p) == '&' ) \
        fputs( "&amp;", (fp) ); \
    else \
        fputc( *(p), (fp) ); \
    (p)++;

#define isSeparator(c)  \
    ( ((c) == ' ')  || \
      ((c) == '\t') || \
      ((c) == '\r') || \
      ((c) == '\n') || \
      ((c) == '+')  || \
      ((c) == '-')  || \
      ((c) == '*')  || \
      ((c) == '/')  || \
      ((c) == '%')  || \
      ((c) == '=')  || \
      ((c) == '|')  || \
      ((c) == '&')  || \
      ((c) == '^')  || \
      ((c) == '<')  || \
      ((c) == '>')  || \
      ((c) == '.')  || \
      ((c) == '!')  || \
      ((c) == '?')  || \
      ((c) == ':')  || \
      ((c) == ';')  || \
      ((c) == '(')  || \
      ((c) == ')')  || \
      ((c) == '[')  || \
      ((c) == ']')  || \
      ((c) == '{')  || \
      ((c) == '}')  || \
      ((c) == ',')     )


int
src2html( FILE *fp, FILE *gp )
{
    char    *p, *q, buf[BUFSIZ];
    char    *rs;
    int     inComment = 0;
    int     inQuote   = 0;
    int     i, found, first, len;
    int     ret = 0;

    fputs( "<blockquote><pre>\n", gp );
    while ( ( p = fgets( buf, BUFSIZ - 1, fp ) ) != NULL ) {
     next:
        if ( !(*p) )
            continue;
        if ( inComment ) {
            q = strstr( p, "*/" );
            if ( q ) {
                char    *r = p;
                while ( r < q + 2 ) {
                    fputString( r, gp );
                }
                fputs( "</span>", gp );
                inComment = 0;
                p = r;
            }
            else {
                while ( *p ) {
                    fputString( p, gp );
                }
                continue;
            }
        }

        if ( inQuote ) {
            char    *r;

            q = strchr( p, '"' );
            if ( q && (*(q - 1) != '\\') ) {
                r = p;
                while ( r < q + 1 ) {
                    fputString( r, gp );
                }
                inQuote = 0;
                p = r;
            }
            else {
                r = p;
                while ( r < q + 1 ) {
                    fputString( r, gp );
                }
                p = r;
                goto next;
            }
        }

        if ( *p == '/' ) {
            if ( *(p + 1) == '/' ) {
                fputs( "<span class=\"comments\">", gp );
                while ( *p ) {
                    fputString( p, gp );
                }
                fputs( "</span>", gp );
                break;
            }

            if ( *(p + 1) == '*' ) {
                fputs( "<span class=\"comments\">/*", gp );
                p += 2;
                inComment = 1;
                goto next;
            }
        }

        first = 1;
        while ( *p ) {
            found = 0;
            while ( isSeparator(*p) ) {
                fputString( p, gp );
                found = 1;
            }
            if ( !(*p) )
                break;
            if ( found || first ) {
                found = 0;
                first = 0;
                for ( rs = reservedString[0], i = 0;
                      reservedString[i] && reservedString[i][0];
                      ++i, rs = reservedString[i] ) {
                    len = strlen( rs );
                    if ( !strncmp( p, rs, len )  &&
                         isSeparator(*(p + len))    ) {
                        fputs( "<span class=\"reservedKeyword\">", gp );
                        fputs( rs, gp );
                        fputs( "</span>", gp );
                        p += strlen(rs);
                        found = 1;
                        break;
                    }
                }
                if ( found )
                    continue;
            }

            if ( *p == '\\' ) {
                if ( (*(p + 1) >= '0') && (*(p + 1) <= '9') ) {
                    fputc( *p, gp );
                    p++;
                    fputc( *p, gp );
                    p++;
                    while ( (*p >= '0') && (*p <= '9') ) {
                        fputc( *p, gp );
                        p++;
                    }
                    continue;
                }
                fputc( *p, gp );
                p++;
                fputc( *p, gp );
                p++;
                continue;
            }

            if ( *p == '\'' ) {
                if ( *(p + 1) == '\\' ) {
                    fputc( *p, gp );
                    p++;
                }
                q = strchr( p + 1, '\'' );
                if ( q ) {
                    while ( p < q + 1 ) {
                        fputString( p, gp );
                    }
                    continue;
                }
                fputc( *p, gp );
                p++;
                continue;
            }

            if ( *p == '"' ) {
                fputc( *p, gp );
                p++;
                inQuote = 1;
                goto next;
            }

            fputString( p, gp );
        }

        ret++;
    }
    fputs( "</pre></blockquote>\n", gp );

    return ( ret );
}
