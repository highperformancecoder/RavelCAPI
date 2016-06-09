rm -rf ravelDoc
latex -interaction=batchmode ravelDoc
if [ $? -ne 0 ]; then exit 1; fi

latex -interaction=batchmode ravelDoc
latex2html -local_icons -info "" -address "" ravelDoc

cp *.png *.xlsx ravelDoc
