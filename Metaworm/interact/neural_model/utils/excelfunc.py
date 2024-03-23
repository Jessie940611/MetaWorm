import xlrd  # version 1.2.0
import numpy as np


def read_excel(filename, sheetname):

	wb = xlrd.open_workbook(filename=filename)
	sheet = wb.sheet_by_name(sheetname)
	nrow = sheet.nrows
	ncol = sheet.ncols
	return sheet, nrow, ncol


def read_excel_array(filename, sheetname):
	con_sht, nrow, ncol = read_excel(filename, sheetname)
	curr_row = 0
	con_mat_ar = np.zeros((nrow-1, ncol-1), dtype=np.int16)
	while curr_row < nrow: # for each row
		row = con_sht.row(curr_row)
		if curr_row > 0: # don't want the first row because those are labels
			for col_ind, el in enumerate(row):
				if col_ind != 0 and len(str(el.value)) > 0:
					con_mat_ar[curr_row - 1, col_ind - 1] = int(el.value)
		curr_row += 1
	return con_mat_ar


if __name__ == "__main__":
	import numpy as np
	filename = "/home/zhaomengdi/Project/eworm/Celegans_as_deep_net_net/chemotaxis_net/components/param/connection/neuron_muscle.xlsx"
	sheetname = "Sheet1"
	con_sht, nrow, ncol = read_excel(filename, sheetname)
	con_mat_ar = read_excel_array(filename, sheetname)
	print(con_mat_ar[23])
	print("con_mat_ar.shape", con_mat_ar.shape)
	print(con_sht.cell_value(0,0), con_sht.cell(1,1).value, type(con_sht.cell(1,1).value), nrow, ncol)
