// FSdata.h, file service data

#ifndef __FS_DATA_H__
#define __FS_DATA_H__

//************************** File Service Data **************************
char help_txt_data[] = {
   'I', '\'', 'm', ' ', 's', 'o', 'r', 'r', 'y', ',', ' ', 'b', 'u', 't',
   ' ', 'm', 'y', ' ', 'c', 'a', 'p', 'a', 'c', 'i', 't', 'y', ' ', 't',
   'o', ' ', 'a', 'n', 's', 'w', 'e', 'r', 'i', 'n', 'g', ' ', 'y', 'o',
   'u', 'r', ' ', 'q', 'u', 'e', 's', 't', 'i', 'o', 'n', 's', ' ', 'i',
   's', ' ', 'l', 'i', 'm', 'i', 't', 'e', 'd', '.', '\n', '\r', 'Y', 'o',
   'u', ' ', 'm', 'u', 's', 't', ' ', 'a', 's', 'k', ' ', 't', 'h', 'e',
   ' ', 'r', 'i', 'g', 'h', 't', ' ', 'q', 'u', 'e', 's', 't', 'i', 'o',
   'n', '.', '\n', '\r', '\0'
};
#define HELP_TXT_SIZE ( sizeof( help_txt_data ) )

char note_txt_data[] = {
   'A', ' ', 'f', 'a', 'i', 'l', 'u', 'r', 'e', ' ', 'i', 's', ' ', 'o',
   'f', 't', 'e', 'n', ' ', 'p', 'r', 'e', 'c', 'u', 'r', 's', 'o', 'r',
   'e', 'd', ' ', 'b', 'y', ' ', '1', '0', '0', ' ', 'e', 'x', 'c', 'u',
   's', 'e', 's', '.', '\n', '\r', '\0'
};
#define NOTE_TXT_SIZE ( sizeof( note_txt_data ) )

// C compiler joins lines below and adds '\0' at the end (not for char arrays above).
char index_html_data[] = {
   "<HTML>\n\r"
   "<HEAD>\n\r"
   "<TITLE>159ers:</TITLE>\n\r"
   "</HEAD>\n\r"
   "<BODY>\n\r"
   "The Few, the Pround. Semper fi!\n\r"
   "Failure's best friend: Excuse.\n\r"
   "Success, on the other hand, has only itself.\n\r"
   "</BODY></HTML>\n\r"
};
#define INDEX_HTML_SIZE ( sizeof( index_html_data ) ) // null is added+counted

char hello_html_data[] = {
   "<HTML><HEAD>\n\r"
   "<TITLE>The Leopard and the Monkeys</TITLE>\n\r"
   "</HEAD>\n\r"
   "<BODY BgColor=#FFFFFF>\n\r"
   "Monkeys in the jungle, they taunt and mob as a lone leopard walking by by "
   "yelping and smugging. The leopard hisses back -- one last warning.\n\r"
   "Monkeys just ignore it, and acerbate their intimidation.\n\r"
   "The leopard strikes, in lightning speed.\n\r"
   "Monkeys flees and cries. Slain and mauled.\n\r"
   "Primemates live in social settings, in each other's eyes. Vanity and hot air.\n\r"
   "A leopard lives only in its own eyes.\n\r"
   "Be a leopard at work, get things done quickly and quietly; not fancy talks nor blame games.\n\r"
   "</BODY></HTML>\n\r"
};
#define HELLO_HTML_SIZE ( sizeof( hello_html_data ) ) // null is added+counted

// We'll define "root_dir[]" later. Here is a forward declare.
extern dir_t root_dir[];                         // prototype it in advance

dir_t bin_dir[] = {
   {16, MODE_DIR, 0, ".", (char *)bin_dir},   // current dir
   {17, MODE_DIR, 0, "..", (char *)root_dir}, // parent dir, forward declared
   {0, 0, 0, NULL, NULL},                      // no entries in dir
   {END_INODE, 0, 0, NULL, NULL}               // end of bin_dir[]
};

dir_t www_dir[] = {
   {10, MODE_DIR, 0, ".", (char *)www_dir},
   {11, MODE_DIR, 0, "..", (char *)root_dir},
   {12, MODE_FILE, HELLO_HTML_SIZE, "hello.html", (char *)hello_html_data},
   {13, MODE_FILE, INDEX_HTML_SIZE, "index.html", (char *)index_html_data},
   {0, 0, 0, NULL, NULL},
   {END_INODE, 0, 0, NULL, NULL}
};

dir_t root_dir[] = {
   {1, MODE_DIR, 0, ".", (char *)root_dir},
   {2, MODE_DIR, sizeof(bin_dir), "bin", (char *)bin_dir},
   {3, MODE_DIR, sizeof(www_dir), "www", (char *)www_dir},
   {4, MODE_FILE, HELP_TXT_SIZE, "help.txt", (char *)help_txt_data},
   {5, MODE_FILE, NOTE_TXT_SIZE, "note.txt", (char *)note_txt_data},
   {0, 0, 0, NULL, NULL},
   {END_INODE, 0, 0, NULL, NULL}
};

fd_t fd_array[FD_NUM];  // one file descriptor for every OPEN_OBJ call
// *********************************************************************

#endif __FS_DATA_H__

