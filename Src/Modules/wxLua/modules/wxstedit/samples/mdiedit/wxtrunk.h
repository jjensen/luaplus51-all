// wxtrunk.h

#if (wxVERSION_NUMBER < 2900)
inline wxString wxJoin(const wxArrayString& as, wxChar sep)
{
   wxString str;

   for (wxArrayString::const_iterator it = as.begin();
        it != as.end();
        it++)
   {
      if (it != as.begin()) str+=sep;
      str+=*it;
   }
   return str;
}
#endif

