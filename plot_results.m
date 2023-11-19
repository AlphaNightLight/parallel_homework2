names = ['matMul'; 'matT'; 'matBlockT'];
time_cols = [4 3 5]; #on the serial dense report

subnames = ['_deep'; '_shallow'; '_square';...
            '_fat'; '_thin'; '_square';...
            '_smallBlock'; '_mediumBlock'; '_largeBlock';];

subname_titles = [', deep vector product'; ', shallow vector product';...
                  ', square matrices';...
                  ', fat matrix'; ', thin matrix'; ', square matrix';...
                  ', divided in small blocks'; ', divided in medium blocks';...
                  ', divided in large blocks';];

for (i_sparsity = 1:2)
  if (i_sparsity == 1)
    sparsity = "_dense";
  else
    sparsity = "_sparse";
  endif
  
  for (i_type = 1:3)
    name = strtrim(names(i_type,:));
    time_col = time_cols(i_type);

    report_serial_full =...
        dlmread(['reports/serial/report_' name sparsity '.csv'], ',', 1, 0);
    report_strong_full =...
        dlmread(['reports/parallel/report_' name 'par' sparsity '_strong.csv'],...
        ',', 1, 0);
    report_weak_full =...
        dlmread(['reports/parallel/report_' name 'par' sparsity '_weak.csv'],...
        ',', 1, 0);

    for (i_subtype=1:3)
      subname = strtrim(subnames( (i_type-1)*3 + i_subtype ,:));
      subname_title = strtrim(subname_titles( (i_type-1)*3 + i_subtype ,:));
      
      ### Extract the values
      if (i_sparsity == 1)
        extractor = i_subtype:3:rows(report_strong_full);
        num_threads = report_strong_full(extractor,1);
        report_strong = report_strong_full(extractor,time_col+1);
        report_weak = report_weak_full(extractor,time_col+1);
        report_serial = ones(length(report_strong),1)...
            * report_serial_full(i_subtype,time_col);
      else
        # in the sparse case, we need an additional +1 over the clumns,
        # as we have the DENSITY column in addition.
        thread_extractor = i_subtype:9:rows(report_strong_full);
        num_threads = report_strong_full(thread_extractor,1);
        
        extractor = i_subtype:3:rows(report_strong_full);
        report_strong = report_strong_full(extractor,time_col+2);
        report_strong = reshape(report_strong, 3, length(report_strong)/3);
        report_weak = report_weak_full(extractor,time_col+2);
        report_weak = reshape(report_weak, 3, length(report_weak)/3);
        
        serial_extractor = i_subtype:3:rows(report_serial_full);
        report_serial = report_serial_full(serial_extractor,time_col+1);
        report_serial = report_serial * ones(1,length(report_strong));
      endif
      
      
      
      ### Strong Plot
      semilogx(num_threads,report_serial,'linewidth',2,...
      num_threads,report_strong,'linewidth',2,'marker','.','markersize',30);

      xticks(num_threads);
      xticklabels(num_threads);
      grid on;
      set(gca,'XMinorTick','Off');
      set (gca, 'xminorgrid', 'off');
      set(gca,'YMinorTick','Off');
      set (gca, 'yminorgrid', 'off');

      title([name '\' sparsity subname_title ', Strong Scaling']);
      ylabel('time [s]');
      xlabel('num\_threads [n]');
      if (i_sparsity == 1)
        legend('Serial', 'Parallel', 'location', 'northwest');
      else
        legend('Serial 0.2','Serial 0.5','Serial 0.8',...
            'Parallel 0.2','Parallel 0.5','Parallel 0.8',...
            'location', 'northwest');
      endif
      print(['plots/' name sparsity '/'...
          'plot_' name sparsity subname '_strong.png'],'-dpng');
      
      
      
      ### Speedup Plot
      speedup_factor = report_strong(:,1) * ones(1,length(report_strong));
      report_strong = speedup_factor ./ report_strong;
      semilogx(num_threads,num_threads,'linewidth',2,...
      num_threads,report_strong,'linewidth',2,'marker','.','markersize',30);

      xticks(num_threads);
      xticklabels(num_threads);
      grid on;
      set(gca,'XMinorTick','Off');
      set (gca, 'xminorgrid', 'off');
      set(gca,'YMinorTick','Off');
      set (gca, 'yminorgrid', 'off');

      title([name '\' sparsity subname_title ', Speedup']);
      ylabel('Speedup [n]');
      xlabel('num\_threads [n]');
      if (i_sparsity == 1)
        legend('Ideal', 'Parallel', 'location', 'northwest');
      else
        legend('Ideal','Parallel 0.2','Parallel 0.5','Parallel 0.8',...
            'location', 'northwest');
      endif
      print(['plots/' name sparsity '/'...
          'plot_' name sparsity subname '_speedup.png'],'-dpng');
      
      
      
      ### Efficiency Plot
      num_threads_temp = num_threads * ones(1,rows(report_strong));
      num_threads_temp = transpose(num_threads_temp);
      report_strong = (report_strong ./ num_threads_temp) * 100;
      ideal_efficiency = ones(length(report_strong),1) * 100;
      semilogx(num_threads,ideal_efficiency,'linewidth',2,...
      num_threads,report_strong,'linewidth',2,'marker','.','markersize',30);

      xticks(num_threads);
      xticklabels(num_threads);
      grid on;
      set(gca,'XMinorTick','Off');
      set (gca, 'xminorgrid', 'off');
      set(gca,'YMinorTick','Off');
      set (gca, 'yminorgrid', 'off');

      title([name '\' sparsity subname_title ', Efficiency']);
      ylabel('Efficiency [%]');
      xlabel('num\_threads [n]');
      if (i_sparsity == 1)
        legend('Ideal', 'Parallel', 'location', 'northwest');
      else
        legend('Ideal','Parallel 0.2','Parallel 0.5','Parallel 0.8',...
            'location', 'northwest');
      endif
      print(['plots/' name sparsity '/'...
          'plot_' name sparsity subname '_efficiency.png'],'-dpng');
      
      
      
      ### Weak Plot
      semilogx(num_threads,report_serial,'linewidth',2,...
      num_threads,report_weak,'linewidth',2,'marker','.','markersize',30);

      xticks(num_threads);
      xticklabels(num_threads);
      grid on;
      set(gca,'XMinorTick','Off');
      set (gca, 'xminorgrid', 'off');
      set(gca,'YMinorTick','Off');
      set (gca, 'yminorgrid', 'off');

      title([name '\' sparsity subname_title ', Weak Scaling']);
      ylabel('time [s]');
      xlabel('num\_threads & size\_scale\_ factor [n]');
      if (i_sparsity == 1)
        legend('Serial with size[1]', 'Parallel', 'location', 'northwest');
      else
        legend('Serial 0.2 with size[1]','Serial 0.5 with size[1]',...
            'Serial 0.8 with size[1]',...
            'Parallel 0.2','Parallel 0.5','Parallel 0.8',...
            'location', 'northwest');
      endif
      print(['plots/' name sparsity '/'...
          'plot_' name sparsity subname '_weak.png'],'-dpng');
    endfor
  endfor
endfor
