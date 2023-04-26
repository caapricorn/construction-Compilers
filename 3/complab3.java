import java.io.File;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/*Комментарии: целиком строка текста, начинающаяся с «*». 
* Идентификаторы: либо последовательности латинских букв нечётной длины, либо последовательности символов «*». 
Ключевые слова: «with», «end», «**». */

public class complab3 {
    public static void main(String args[]) {

        String text = "";
        Scanner scanner;

        try {
            scanner = new Scanner(new File("/home/caapricorn/Рабочий стол/kk/3/in.txt"));
        } catch (java.io.FileNotFoundException e) {
            System.out.println(e.toString());
            return;
        }

        System.out.println("cols - [123456789]");
        int linenum = 1;
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            text += line + "\n";
            System.out.println("line " + linenum + " [" + line + "]");
            linenum++;
        }

        //Комментарии: начинаются с «(∗» или «{», заканчиваются на «∗)» или «}»
        // и могут пересекать границы строк текста.
        String comment_1 = "(\\*\n$)";
        //String comment_2 = "(\\(\\*([^\\*]|\\*($|[^\\)]))*\\*\\))";
        // exclude "not|this": ^([^nt]|n($|[^o]|o($|[^t]))|t($|[^h]|h($|[^i]|i($|[^s]))))*$

        //Идентификаторы: последовательности латинских букв,
        // представляющие собой конкатенации двух одинаковых слов («zz», «abab»).
        String ident = "(([A-Za-z]+)\\10)";

        //Ключевые слова: «with», «end», «**».
        String keyword = "(with|end|\\*\\*)";

        String whitespace = "(\r| |\t)";

        String linebreak = "\n";



        String pattern = "(^" + comment_1 +       // 1 2
                        ")|(^" + keyword +                           // 3 4
                        ")|(^" + ident +                             // 5 6 7
                        ")|(^" + whitespace +                        // 8 9 
                        ")|(^" + linebreak + ")";                    // 10

        Pattern p = Pattern.compile(pattern);

        int line = 1, linepos = 1;
        boolean err = false;



        while (!text.equals("")) {

            Matcher m = p.matcher(text);
            if (m.find()) {
                err = false;

                if (null != m.group(1)) {
                    System.out.println(m.group(1));
                    String tmp = m.group(1);

                    System.out.println("COMMENT (" + line + ", " + linepos + "): " + tmp.replaceAll("\n", " "));

                    while (tmp.contains("\n")) {
                        line++;
                        linepos = 1;
                        tmp = tmp.substring(tmp.indexOf("\n") + 1);
                    }
                    linepos += tmp.length();
                    text = text.substring(m.end());
                    continue;
                }

                if (null != m.group(3))
                    System.out.println("KEYWORD (" + line + ", " + linepos + "): " + m.group(3));

                if (null != m.group(5))
                    System.out.println("IDENT (" + line + ", " + linepos + "): " + m.group(5));



                if (null != m.group(10)) {
                    System.out.println(m.group(10));
                    //System.out.println("lb (" + line + ", " + linepos + ")");
                    line++;
                    linepos = 0;
                }

                //if (null != m.group(11))
                //    System.out.println("ws (" + line + ", " + linepos + ")");


                text = text.substring(m.end());
                linepos += m.end();

            } else {

                if (true != err) {
                    System.out.println("SYNTAX ERR (" + line + ", " + linepos + ")");
                    err = true;
                }

                text = text.substring(1);
                linepos++;
            }


        }

        System.out.println("\ndone");
    }

}