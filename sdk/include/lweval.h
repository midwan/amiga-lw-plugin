/* lweval.h -- Header for Global Equation.p, an expression evaluator */
/* Wed Feb 15 00:11:45 1995 */
/* Fri May 13 1995 */

typedef struct {
	char		*symbol;
	double	value;
} ev_arg;

/* Returned by Global activation, for ID string "ExpressEval" */
typedef	struct st_Evaluator {
	int			MaxInput;												/* Read-only, longest string accepted by f'ns below... */
	double	(*StringValue)(char *); 					/* Return Expression value */
	double	(*ValueXYZT)(char *,double,double,double,double);			/* Return Expression value with var.s 'x', 'y', 'z', and 't' filled by doubles */
	double	(*Value)(char *,int, ev_arg *);					/*	Return expression with n args */
} Evaluator;

