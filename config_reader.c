//
// created by souranil on 2/1/2026.
//
int64_t index(const char *str, const char search_term){
    /*This function searches for a particular search_term char in the given string
    * returns -1 on failure to find the search_term or index of the char in the string.
    * In order for this function to work properly the given string has to null-terminated.
    */
    int64_t total_len=strlen(str);
    char *idx=strchr(str, search_term);
    if (idx==NULL){ //Search term doesn't exist in the string
        return -1;
    }
    int64_t split_len=strlen(idx);
    return (int64_t) total_len-split_len;
}
int split_in_two(const char *src, char * target_buffer[2], const char splitter){
    /*
    * Split the given src string in two parts at the splitter char idx. This function does check if the splitter
    * char is in the string or not. 
    */
    char *split_idx=strchr(src, splitter);
    if (split_idx==NULL && split_idx==src){
        return -1;
    }
    split_idx[0]='\0';
    target_buffer[0]=strdup(src);
    target_buffer[1]=strdup(split_idx+1);
    if (target_buffer[0]==NULL || target_buffer[1]==NULL){
        return -1;
    }
    return 0;
}
