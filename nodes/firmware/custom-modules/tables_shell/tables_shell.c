#include "shell.h"
#include "tables.h"
#include "tables/records.h"

/* the tables instance used must be defined by the application */
extern tables_context_t *tables;

int sc_tables(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    TABLE_ITERATOR(iter, tables);
    table_query_t all_records_query;
    tables_init_query(&all_records_query, RECORD_UNDEFINED, NULL, NULL);

    int res = tables_iterator_init(tables, &iter, &all_records_query);
    if (res) {
        printf("%s iter init failed (%d)\n", __func__, res);
        return -1;
    }

    table_record_t *record;
    char record_str[TABLE_RECORD_STRING_SIZE];
    int record_cnt = 0;

    while( tables_iterator_next(tables, &iter, &record, NULL, NULL) == 0) {
        record_tostr(record, record_str, sizeof(record_str));
        printf("%d: %s\n", record_cnt, record_str);
        record_cnt++;
    }

    return 0;
}

SHELL_COMMAND(tables, "print tables", sc_tables);
