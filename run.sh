make
./smp_cache 8192 8 64 4 0 ./Validation_runs/canneal.04t.debug > ../msid
./smp_cache 8192 8 64 4 0 ./Validation_runs/canneal.04t.longTrace > ../msil
./smp_cache 8192 8 64 4 0 ./Validation_runs/blacksholes.04t.debug > ../msib
./smp_cache 8192 8 64 4 1 ./Validation_runs/canneal.04t.debug > ../mesid
./smp_cache 8192 8 64 4 1 ./Validation_runs/canneal.04t.longTrace > ../mesil
./smp_cache 8192 8 64 4 1 ./Validation_runs/blacksholes.04t.debug > ../mesib
./smp_cache 8192 8 64 4 2 ./Validation_runs/canneal.04t.debug > ../moesid
./smp_cache 8192 8 64 4 2 ./Validation_runs/canneal.04t.longTrace > ../moesil
./smp_cache 8192 8 64 4 2 ./Validation_runs/blacksholes.04t.debug > ../moesib
diff ../msid ./Validation_runs/MSI_c_4.out
diff ../msil ./Validation_runs/MSI_c_4.long.out
diff ../msib ./Validation_runs/MSI_b_4.out
diff ../mesid ./Validation_runs/MESI_c_4.out
diff ../mesil ./Validation_runs/MESI_c_4.long.out
diff ../mesib ./Validation_runs/MESI_b_4.out
diff ../moesid ./Validation_runs/MOESI_c_4.out
diff ../moesil ./Validation_runs/MOESI_c_4.long.out
diff ../moesib ./Validation_runs/MOESI_b_4.out


