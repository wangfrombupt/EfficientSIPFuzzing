
DIR="CVS_"`date +%Y.%m.%d_%H:%M:%S`

export CVSROOT=:pserver:anonymous@cvs.berlios.de:/cvsroot/ser
cvs login
cvs export -r HEAD sip_router . 

mv -v sip_router $DIR

svn add $DIR 

cd $DIR

# Step 1 - convert .cvsignore

find . -name ".cvsignore" | while read file; do
    svn propset svn:ignore "`cat "$file"`" "`echo "$file" | sed 's,/[^/]*$,,'`"
    svn del $file --force
 done

# Step 2 - set executable flags
find . -name "*.sh" -o -name "*.cmd" | while read file; do
    svn propset svn:executable "true" $file
    chmod u+x $file
done

# Step 3 - set expandable keywords
svn propset svn:keywords "URL HeadURL Author LastChangedBy Date LastChangedDate Rev Revision LastChangedRevision Id" -R .